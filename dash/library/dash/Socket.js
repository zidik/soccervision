Dash.Socket = function(socket) {
	this.host = null;
	this.port = null;
	this.ws = null;
	this.opening = false;
	this.socketId = socket.socketId;
	this.socketEvents = socket.Events;
	this.socketStates = socket.States;
};

Dash.Socket.prototype = new Dash.Bindable();
/*
Dash.Socket.Event = {
	OPEN: 'open',
	CLOSE: 'close',
	MESSAGE_RECEIVED: 'message-received',
	MESSAGE_SENT: 'message-sent',
	ERROR: 'error'
};
Dash.Socket.Event2 = {
	OPEN: 'open2',
	CLOSE: 'close2',
	MESSAGE_RECEIVED: 'message-received2',
	MESSAGE_SENT: 'message-sent2',
	ERROR: 'error2'
};
Dash.Socket.State = {
	CONNECTING: 0,
	OPEN: 1,
	CLOSING: 2,
	CLOSED: 3
};
 */
Dash.Socket.prototype.open = function(host, port, events) {
	var self = this;

	this.host = host;
	this.port = port;

	this.ws = new WebSocket('ws://' + this.host + ':' + this.port);
	this.opening = true;
	
	this.ws.onopen = function() {
		this.opening = false;

			self.fire({
				type: self.socketEvents.OPEN,
				socket: self
			});

	};


	this.ws.onclose = function() {
		self.fire({
			type: self.socketEvents.CLOSE,
			socket: self
		});
	};
	
	this.ws.onerror = function(message) {
		self.fire({
			type: self.socketEvents.ERROR,
			message: message,
			socket: self
		});
	};

	this.ws.onmessage = function(message) {

			self.fire({
				type: self.socketEvents.MESSAGE_RECEIVED,
				message: message
			});



	};
};

Dash.Socket.prototype.getState = function() {
	if (this.ws == null) {
		return this.socketStates.CLOSED;
	} else {
		return this.ws.readyState;
	}
};

Dash.Socket.prototype.isOpen = function() {
	return this.getState() == this.socketStates.OPEN;
};

Dash.Socket.prototype.getBufferedAmount = function() {
	if (this.ws == null) {
		return 0;
	} else {
		return this.ws.bufferedAmount;
	}
};

Dash.Socket.prototype.send = function(message) {

	if (this.ws != null) {
		var state = this.getState();
		
		if (state != this.socketStates.OPEN) {
			this.fire({
				type: this.socketEvents.ERROR,
				message: 'Unable to send message "' + message.replace('<', '&lt;').replace('>', '&gt;') + '", socket is in invalid state #' + state,
				socket: this
			});
			
			return;
		}
		this.ws.send(message);

		this.fire({
			type: this.socketEvents.MESSAGE_SENT,
			message: message
		});
	}
};

Dash.Socket.prototype.close = function() {
	if (this.ws != null) {
		this.ws.close();
		this.ws = null;
	}
};