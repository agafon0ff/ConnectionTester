/* String variable */
var valueString = 'hello';

/* Uint8Array variable */
var valueArray = new Array(0x68, 0x65, 0x6c, 0x6c, 0x6f);

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
	var data = new Array(0x68, 0x65, 0x6c, 0x6c, 0x6f);
	writeData(data, '127.0.0.1', 12345);
}
