
var g = g || {};
g.config = g.config || {};

let client = new Client();

g.vue = new Vue({
    el : '#msg-app',
    data : {
	messages : [],
	message : '',
    },
    methods : {
	SendMessage : function() {
            client.SendMessage(this.message);
	}
    }
});

g.messages = g.vue.messages;

function SerializationTest()
{
    protobuf.load("proto/message.proto").then(function(root) {
	var GreetMessage = root.lookupType("game.GreetMessage");
	var msg = GreetMessage.create({Text : "Hello from client"});

	var buffer = GreetMessage.encode(msg).finish();

	console.log(buffer);
    });
}