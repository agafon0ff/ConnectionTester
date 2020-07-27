/* String variable */
var valueString = 'hello';

/* Uint8Array variable */
var valueArray = [0x68, 0x65, 0x6c, 0x6c, 0x6f];

/* Read text from socket */
function onReadText(text, host, port)
{
	console.log('Message (' + host + ':' + port + '): ' + text);
}

/* Read binary from socket */
function onReadData(data, host, port)
{
	console.log('Data (' + host + ':' + port + '): ' + data);
}

/* Start script function */
function onScriptStart()
{
	console.log('Script started!');
	scriptStarted(); //changes the status of the script to "Run"
}

/* Stop script function */
function onScriptStop()
{
	console.log('Script stopped!');
	scriptStopped(); //changes the status of the script to "Stopped"
}

/* Write text to socket */
function onScriptStart()
{
	writeText('hello', '127.0.0.1', 12345);
}

/* Write binary to socket */
function onScriptStart()
{
    var data = [0x68, 0x65, 0x6c, 0x6c, 0x6f];
	writeData(data, '127.0.0.1', 12345);
}

/* Timeout */
var timeoutId = 0;

function printHello()
{
    console.log('hello!');
    scriptStopped();
}

function onScriptStart()
{
    timeoutId = setTimeout(printHello, 1500);
    scriptStarted();
}

function onScriptStop()
{
    clearTimeout(timeoutId);
    scriptStopped();
}

/* Interval */
var intervalId = 0;

function printHello()
{
    console.log('hello!');
}

function onScriptStart()
{
    intervalId = setInterval(printHello, 500);
    scriptStarted();
}

function onScriptStop()
{
    clearInterval(intervalId);
    scriptStopped();
}

/* JSON to Object */
var jsonString = '\
{\
    "string": "sample",\
    "number": 12345,\
    "bolean": true,\
    "object": {"qwe1": "hello", "qwe2": "world"},\
    "array": [101, 102, 103, 104]\
}';


function onScriptStart()
{
    var obj = JSON.parse(jsonString);

    var allKeys = Object.keys(obj);

    for(var i=0; i<allKeys.length; ++i)
        console.log(allKeys[i] + ': ' + obj[allKeys[i]]);

    console.log(obj.object.qwe1 + " " + obj.object.qwe2);
    console.log(obj.array[0] + ", " +  obj.array[3]);
}

/* Object to JSON */
var tmpObject =
{
    string: "sample",
    number: 12345,
    bolean: true,
    object: {qwe1: "hello", qwe2: "world"},
    array: [101, 102, 103, 104]
};

function onScriptStart()
{
    var jsonText = JSON.stringify(tmpObject);
    console.log(jsonText);
}
