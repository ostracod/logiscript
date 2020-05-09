
let fs = require("fs");
let net = require("net");
let pathUtils = require("path");
let childProcess = require("child_process");

if (process.argv.length !== 3) {
    console.log("Please provide a path to a LogiScript interpreter.");
    process.exit(1);
}

let interpreterPath = process.argv[2];
let socketPath = pathUtils.join(__dirname, "testSocket");
let testSuiteDirectory = pathUtils.join(__dirname, "testSuites");
let testModuleDirectory = pathUtils.join(__dirname, "testModule");
let tracePosRegex = /^From line (\d+) of (.+)$/;

let socketServer;
let interpreterProcess;
let interpreterClient;
let interpreterHasExited;
let receivedSocketData;
let receivedStdoutData;
let stdoutLineQueue;
let activeTestCase;

let testSuitePathSet = [];
let tempNameList = fs.readdirSync(testSuiteDirectory);
for (let name of tempNameList) {
    testSuitePathSet.push(pathUtils.join(testSuiteDirectory, name));
}

function handleReceivedPacket(data) {
    let tempText = data.toString();
    let endIndex = seekCharacter(tempText, 0, " ");
    let tempCommand = tempText.substring(0, endIndex);
    let tempOperand;
    if (endIndex < tempText.length) {
        tempOperand = tempText.substring(endIndex + 1, tempText.length);
    } else {
        tempOperand = null;
    }
    switch (tempCommand) {
        case "entryPoint":
        {
            sendTextPacket(activeTestCase.entryPointPath);
            break;
        }
        case "readFile":
        {
            sendTextPacket(activeTestCase.fileContentMap[tempOperand]);
            break;
        }
        default:
        {
            throw new Error(`Unknown socket command "${tempCommand}".`);
        }
    }
}

function sendPacket(data) {
    let tempBuffer = Buffer.alloc(4);
    tempBuffer.writeInt32LE(data.length, 0);
    interpreterClient.write(tempBuffer);
    interpreterClient.write(data);
}

function sendTextPacket(text) {
    sendPacket(Buffer.concat([Buffer.from(text), Buffer.from([0])]));
}

function socketReceiveEvent() {
    let contentOffset = 4;
    if (receivedSocketData.length < contentOffset) {
        return;
    }
    let tempLength = receivedSocketData.readInt32LE(0);
    let endIndex = contentOffset + tempLength;
    if (receivedSocketData.length < endIndex) {
        return;
    }
    let packetData = receivedSocketData.slice(contentOffset, endIndex);
    receivedSocketData = receivedSocketData.slice(endIndex, receivedSocketData.length);
    handleReceivedPacket(packetData);
}

function stdoutReceiveEvent() {
    let startIndex = 0;
    for (let index = 0; index < receivedStdoutData.length; index++) {
        if (receivedStdoutData[index] == 10) {
            let tempLine = receivedStdoutData.slice(startIndex, index).toString();
            stdoutLineQueue.push(tempLine);
            startIndex = index + 1;
        }
    }
    if (startIndex > 0) {
        receivedStdoutData = receivedStdoutData.slice(startIndex, receivedStdoutData.length);
    }
}

function closeSocketServer() {
    socketServer.close(() => {
        console.log("Server finished running.");
    });
}

function launchInterpreter() {
    
    receivedSocketData = Buffer.alloc(0);
    receivedStdoutData = Buffer.alloc(0);
    stdoutLineQueue = [];
    interpreterHasExited = false;
    
    interpreterProcess = childProcess.spawn(
        interpreterPath,
        ["--socket", socketPath]
    );
    
    interpreterProcess.stdout.on("data", data => {
        receivedStdoutData = Buffer.concat([receivedStdoutData, data]);
        stdoutReceiveEvent();
    });
    
    interpreterProcess.on("close", () => {
        interpreterHasExited = true;
    });
}

function skipWhitespace(text, index) {
    while (index < text.length) {
        if (text.charAt(index) !== " ") {
            break;
        }
        index += 1;
    }
    return index;
}

function seekCharacter(text, index, character) {
    while (index < text.length) {
        if (text.charAt(index) === character) {
            break;
        }
        index += 1;
    }
    return index;
}

function parseTerm(text, index) {
    let startIndex;
    let endIndex;
    if (text.charAt(index) === "\"") {
        index += 1;
        startIndex = index;
        index = seekCharacter(text, index, "\"");
        endIndex = index;
        index += 1;
    } else {
        startIndex = index;
        index = seekCharacter(text, index, " ");
        endIndex = index;
    }
    return {
        term: text.substring(startIndex, endIndex),
        index: index
    };
}

function parseTestDirective(text) {
    if (text.length < 3 || text.substring(0, 3) !== "///") {
        return null;
    }
    text = text.substring(3, text.length);
    let output = [];
    let index = 0;
    while (index < text.length) {
        index = skipWhitespace(text, index);
        if (index >= text.length) {
            break;
        }
        let tempResult = parseTerm(text, index);
        output.push(tempResult.term);
        index = tempResult.index;
    }
    return output;
}

function seekEndDirective(lineList, index) {
    for (; index < lineList.length; index++) {
        let tempLine = lineList[index];
        let tempDirective = parseTestDirective(tempLine);
        if (tempDirective === null) {
            continue;
        }
        if (tempDirective[0] === "END") {
            break;
        }
    }
    return index;
}

// Invokes timerEvent with resolve and reject arguments on a timer.
// When timerEvent invokes resolve or reject, the timer is cleared.
function promiseInterval(delay, timerEvent) {
    let interval;
    return (new Promise((resolve, reject) => {
        function invokeTimerEvent() {
            timerEvent(resolve, reject);
        }
        interval = setInterval(invokeTimerEvent, delay);
        invokeTimerEvent();
    })).finally(() => {
        clearInterval(interval);
    });
}

class InterpreterStateError extends Error {
    
}

function promiseIntervalWithTimeout(
    operationName,
    timerEvent,
    delay = 20,
    timeoutCount = 20
) {
    let hasFinishedPromise = false;
    return promiseInterval(delay, (resolve, reject) => {
        timerEvent(resolve, reject);
        if (hasFinishedPromise) {
            return;
        }
        timeoutCount -= 1;
        if (timeoutCount <= 0) {
            reject(new InterpreterStateError(operationName + " operation timed out."));
        }
    }).finally(() => {
        hasFinishedPromise = true;
    });
}

function waitForInterpreterProcessToExit() {
    return promiseIntervalWithTimeout("Exit", (resolve, reject) => {
        if (interpreterHasExited) {
            resolve();
        }
    });
}

class RuntimeAction {
    constructor() {
        this.hasSucceeded = false;
    }
}

class ReadLineAction extends RuntimeAction {
    
    constructor(expectedText) {
        super();
        this.receivedText = null;
    }
    
    perform() {
        return promiseIntervalWithTimeout("Read", (resolve, reject) => {
            if (stdoutLineQueue.length > 0) {
                this.receivedText = stdoutLineQueue.shift();
                this.checkReceivedText();
                resolve();
            } else if (interpreterHasExited) {
                reject(new InterpreterStateError("Interpreter exited unexpectedly."));
            }
        });
    }
}

class ExpectOutputAction extends ReadLineAction {
    
    constructor(expectedText) {
        super();
        this.expectedText = expectedText;
    }
    
    checkReceivedText() {
        this.hasSucceeded = (this.expectedText === this.receivedText);
    }
    
    getFailureMessage() {
        if (this.receivedText === null) {
            return `Expected stdout "${this.expectedText}", did not receive message.`;
        } else {
            return `Expected stdout "${this.expectedText}", received "${this.receivedText}".`;
        }
    }
}

class ExpectTracePosAction extends ReadLineAction {
    
    constructor(expectedLineNumber, expectedPath) {
        super();
        this.expectedLineNumber = expectedLineNumber;
        this.expectedPath = expectedPath;
        this.receivedLineNumber = null;
        this.receivedPath = null;
    }
    
    checkReceivedText() {
        let tempResult = this.receivedText.match(tracePosRegex);
        if (tempResult === null) {
            this.hasSucceeded = false;
            return;
        }
        this.receivedLineNumber = parseInt(tempResult[1]);
        this.receivedPath = tempResult[2];
        this.hasSucceeded = (this.expectedLineNumber === this.receivedLineNumber
            && this.expectedPath === this.receivedPath);
    }
    
    getFailureMessage() {
        let output = `Expected stack trace pos on line ${this.expectedLineNumber} of ${this.expectedPath}, `;
        if (this.receivedText === null) {
            output += "did not receive message.";
        } else {
            output += "received:\n" + this.receivedText;
        }
        return output;
    }
}

class ProvideInputAction extends RuntimeAction {
    
    constructor(text) {
        super();
        this.text = text;
    }
    
    perform() {
        return promiseIntervalWithTimeout("Prompt", (resolve, reject) => {
            if (receivedStdoutData.length >= 2) {
                if (receivedStdoutData[0] === 62 && receivedStdoutData[1] === 32) {
                    receivedStdoutData = receivedStdoutData.slice(
                        2,
                        receivedStdoutData.length
                    );
                    interpreterProcess.stdin.write(this.text + "\n");
                    this.hasSucceeded = true;
                    resolve();
                } else {
                    reject(new InterpreterStateError("Expected stdin prompt."));
                }
            } else if (interpreterHasExited) {
                reject(new InterpreterStateError("Interpreter exited unexpectedly."));
            }
        });
    }
    
    getFailureMessage() {
        return "Failed to provide input text.";
    }
}

class TestCase {
    
    constructor(name) {
        this.name = name;
        // Map from file path to content.
        this.fileContentMap = {};
        this.entryPointPath = null;
        this.runtimeActionList = [];
        this.expectedExitCode = null;
        this.isExpectingStackTrace = false;
        this.interpreterHasStateError = false;
        this.hasSucceeded = false;
    }
    
    handleInterpreterStateError(error) {
        if (error instanceof InterpreterStateError) {
            console.log(error.message);
            this.interpreterHasStateError = true;
        } else {
            throw error;
        }
    }
    
    perform() {
        console.log("Running test case: " + this.name);
        activeTestCase = this;
        launchInterpreter();
        return this.runtimeActionList.reduce((accumulator, runtimeAction) => {
            return accumulator.then(() => runtimeAction.perform());
        }, Promise.resolve()).catch((error) => {
            this.handleInterpreterStateError(error);
        }).then(
            waitForInterpreterProcessToExit
        ).catch((error) => {
            this.handleInterpreterStateError(error);
            interpreterProcess.kill();
            return waitForInterpreterProcessToExit();
        }).then(() => {
            let tempHasSucceeded = true;
            for (let runtimeAction of this.runtimeActionList) {
                if (!runtimeAction.hasSucceeded) {
                    console.log(runtimeAction.getFailureMessage());
                    tempHasSucceeded = false;
                }
            }
            if (this.interpreterHasStateError) {
                tempHasSucceeded = false;
            }
            if (interpreterProcess.signalCode !== null) {
                console.log(`Interpreter exited with ${interpreterProcess.signalCode} signal.`);
                tempHasSucceeded = false;
            } else if (interpreterProcess.exitCode !== this.expectedExitCode) {
                console.log(`Expected exit code ${this.expectedExitCode}, but received ${interpreterProcess.exitCode}.`);
                tempHasSucceeded = false;
            }
            let hasReceivedUnexpectedStdout = false;
            if (this.isExpectingStackTrace) {
                if (stdoutLineQueue.length <= 0) {
                    console.log("Expected stack trace.");
                    tempHasSucceeded = false;
                }
                for (let line of stdoutLineQueue) {
                    let tempResult = line.match(tracePosRegex);
                    if (tempResult === null) {
                        hasReceivedUnexpectedStdout = true;
                        break;
                    }
                }
            } else if (stdoutLineQueue.length > 0) {
                hasReceivedUnexpectedStdout = true;
            }
            if (hasReceivedUnexpectedStdout) {
                console.log("Received unexpected stdout:");
                console.log(stdoutLineQueue.join("\n"));
                tempHasSucceeded = false;
            }
            this.hasSucceeded = tempHasSucceeded;
        });
    }
}

class TestSuite {
    
    constructor(path) {
        this.path = path;
        this.failureCount = 0;
        this.testCaseList = [];
        let currentTestCase = null;
        let lineList = fs.readFileSync(this.path, "utf8").split("\n");
        for (let index = 0; index < lineList.length; index++) {
            let tempLine = lineList[index];
            let tempDirective = parseTestDirective(tempLine);
            if (tempDirective === null) {
                continue;
            }
            let tempCommand = tempDirective[0];
            switch (tempCommand) {
                case "CASE":
                {
                    currentTestCase = new TestCase(tempDirective[1]);
                    this.testCaseList.push(currentTestCase);
                    break;
                }
                case "FILE":
                {
                    index += 1;
                    let startIndex = index;
                    let endIndex = seekEndDirective(lineList, index);
                    index = endIndex;
                    let tempContent = lineList.slice(startIndex, endIndex).join("\n");
                    let tempPath = pathUtils.join(
                        testModuleDirectory,
                        tempDirective[1]
                    );
                    currentTestCase.fileContentMap[tempPath] = tempContent;
                    break;
                }
                case "RUN":
                {
                    currentTestCase.entryPointPath = pathUtils.join(
                        testModuleDirectory,
                        tempDirective[1]
                    );
                    break;
                }
                case "EXPECT_OUTPUT":
                {
                    index += 1;
                    let startIndex = index;
                    let endIndex = seekEndDirective(lineList, index);
                    index = endIndex;
                    for (let tempIndex = startIndex; tempIndex < endIndex; tempIndex++) {
                        let tempAction = new ExpectOutputAction(lineList[tempIndex]);
                        currentTestCase.runtimeActionList.push(tempAction);
                    }
                    break;
                }
                case "PROVIDE_INPUT":
                {
                    let tempAction = new ProvideInputAction(tempDirective[1]);
                    currentTestCase.runtimeActionList.push(tempAction);
                    break;
                }
                case "EXPECT_TRACE_POS":
                {
                    let tempAction = new ExpectTracePosAction(
                        parseInt(tempDirective[1]),
                        pathUtils.join(testModuleDirectory, tempDirective[2])
                    );
                    currentTestCase.runtimeActionList.push(tempAction);
                    break;
                }
                case "EXPECT_STACK_TRACE":
                {
                    currentTestCase.isExpectingStackTrace = true;
                    break;
                }
                case "EXPECT_EXIT_CODE":
                {
                    currentTestCase.expectedExitCode = parseInt(tempDirective[1]);
                    break;
                }
                default:
                {
                    throw new Error(`Unknown directive command "${tempCommand}".`);
                }
            }
        }
    }
    
    perform() {
        console.log("\nRunning test suite: " + this.path);
        return this.testCaseList.reduce((accumulator, testCase) => {
            return accumulator.then(() => testCase.perform());
        }, Promise.resolve()).then(() => {
            for (let testCase of this.testCaseList) {
                if (!testCase.hasSucceeded) {
                    this.failureCount += 1;
                }
            }
            console.log(`Test suite failures: ${this.failureCount} / ${this.testCaseList.length}`);
        });
    }
}

function performAllTestSuites() {
    let testSuiteList = testSuitePathSet.map(path => new TestSuite(path));
    return testSuiteList.reduce((accumulator, testSuite) => {
        return accumulator.then(() => testSuite.perform());
    }, Promise.resolve()).then(() => {
        console.log("\nFinished running all tests suites.");
        let failureCount = 0;
        let testCaseCount = 0;
        for (let testSuite of testSuiteList) {
            failureCount += testSuite.failureCount;
            testCaseCount += testSuite.testCaseList.length;
        }
        console.log(`TOTAL FAILURES: ${failureCount} / ${testCaseCount}`);
    });
}

if (fs.existsSync(socketPath)) {
    fs.unlinkSync(socketPath);
}

socketServer = net.createServer(client => {
    
    interpreterClient = client;
    
    client.on("data", data => {
        receivedSocketData = Buffer.concat([receivedSocketData, data]);
        socketReceiveEvent();
    });
});

socketServer.listen(socketPath, () => {
    console.log("Listening to socket.");
    performAllTestSuites().then(closeSocketServer);
});


