class ConnectionHandler
{
    ConnectionHandler()
    {
	this.Connection = null;
    }

    InitializeConnection(Connection)
    {
	this.Connection = Connection;
    }

    OnConnected()
    {
	console.log("Base.Connected");
    }

    OnDisconnected()
    {
	console.log("Base.Disconnected");
    }

    OnError(err)
    {
	console.log("Base.OnError: " + err)
    }

    OnMessage(data)
    {
	console.log("Base.OnMessage: " + data);
    }

    SendMessage(message)
    {
	this.Connection.connection.send(message);
    }
}

class Connection
{
    constructor(address, port, resource, connectionHandler)
    {
	this.address = address;
	this.port = port;
	this.resource = resource;
	this.fullAddress = this.address + ":" + this.port + "/" + this.resource;
	this.connectionHandler = connectionHandler || new ConnectionHandler();
	this.connection = null;
    }

    Connect()
    {
	var self = this;
	if("WebSocket" in window) {
	    var ws = new WebSocket(this.fullAddress);

	    ws.onopen = function() {
		self.connectionHandler.OnConnected();
	    };

	    ws.onmessage = function(evt) {
		self.connectionHandler.OnMessage(evt.data);
	    };

	    ws.onclose = function(evt) {
		self.connectionHandler.OnDisconnected();
	    };

	    ws.onerror = function(evt) {
		self.connectionHandler.OnError();
	    };

	    self.connection = ws;
	    self.connectionHandler.InitializeConnection(self.connection);
	} else {
	    alert("WebSockets NOT supported by your Browser!");
	}
    }

    Disconnect()
    {
	this.connection.close();
    }
}