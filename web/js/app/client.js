
class ClientConnectionHandler extends ConnectionHandler
{
    constructor()
    {
	super();
    }

    // Getter
    OnConnected()
    {
	console.log("Base.Connected");
    }
    // Method
    OnError()
    {
	alert("error occurred");
    }

    OnMessage(data)
    {
	g.messages.push(data);
    }

    OnDisconnected()
    {
	alert("disconnected");
    }
}

class Client extends ConnectionHandler
{
    constructor()
    {
	super();
	this.Connection = new Connection('ws://localhost', '8082', 'chat',
	                                 new ClientConnectionHandler());
	this.Connection.Connect();
    }
}
