
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

let socketServer;
let interpreterProcess;
let interpreterClient = null;
let interpreterExitCode = null;
let receivedSocketData = Buffer.alloc(0);
let receivedStdoutData = Buffer.alloc(0);
let stdoutLineQueue = [];
let activeTestCase = null;

let testSuitePathSet = [];
let tempNameList = fs.readdirSync(testSuiteDirectory);
for (let name of tempNameList) {
    testSuitePathSet.push(pathUtils.join(testSuiteDirectory, name));
}

function handleReceivedPacket(data) {
    let tempText = data.toString();
    console.log(`Received packet: "${tempText}"`);
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
    console.log("Sending packet: " + data);
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
            console.log("Received line on stdout: " + tempLine);
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
    
    console.log("Launching interpreter....");
    interpreterExitCode = null;
    interpreterProcess = childProcess.spawn(
        interpreterPath,
        ["--socket", socketPath]
    );
    
    interpreterProcess.stdout.on("data", data => {
        receivedStdoutData = Buffer.concat([receivedStdoutData, data]);
        stdoutReceiveEvent();
    });
    
    interpreterProcess.on("close", code => {
        console.log(`Interpreter exited with code ${code}`);
        interpreterExitCode = code;
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

class RuntimeAction {
    
}

class ExpectOutputAction extends RuntimeAction {
    
    constructor(text) {
        super();
        this.text = text;
    }
}

class TestCase {
    
    constructor(name) {
        this.name = name;
        // Map from file path to content.
        this.fileContentMap = {};
        this.entryPointPath = null;
        this.runtimeActionList = [];
        this.expectedExitCode = [];
    }
    
    perform() {
        console.log("Running test case: " + this.name);
        activeTestCase = this;
        launchInterpreter();
        return this.runtimeActionList.reduce((accumulator, runtimeAction) => {
            return accumulator.then(() => this.performRuntimeAction(runtimeAction));
        }, Promise.resolve());
    }
    
    performRuntimeAction(runtimeAction) {
        return new Promise((resolve, reject) => {
            // TODO: Perform the action and resolve or reject.
            
        });
    }
}

class TestSuite {
    
    constructor(path) {
        this.path = path;
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
    
    performAllTestCases() {
        console.log("Running test suite: " + this.path);
        return this.testCaseList.reduce((accumulator, testCase) => {
            return accumulator.then(() => testCase.perform());
        }, Promise.resolve());
    }
}

function performAllTestSuites() {
    return testSuitePathSet.reduce((accumulator, path) => {
        return accumulator.then(() => {
            let testSuite = new TestSuite(path);
            return testSuite.performAllTestCases();
        });
    }, Promise.resolve()).then(() => {
        console.log("Finished running all tests suites.");
    });
}

if (fs.existsSync(socketPath)) {
    fs.unlinkSync(socketPath);
}

socketServer = net.createServer(client => {
    console.log("Interpreter connected to socket.");
    interpreterClient = client;
    
    client.on("data", data => {
        receivedSocketData = Buffer.concat([receivedSocketData, data]);
        socketReceiveEvent();
    });
    
    client.on("end", () => {
        console.log("Interpreter disconnected from socket.");
    });
});

socketServer.listen(socketPath, () => {
    console.log("Listening to socket.");
    performAllTestSuites().then(closeSocketServer);
});


