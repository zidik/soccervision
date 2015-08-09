Dash.Socket = function(socketId) {
	this.host = null;
	this.port = null;
	this.ws = null;
	this.opening = false;
	this.socketId = socketId;
};

Dash.Socket.prototype = new Dash.Bindable();

Dash.Socket.Event = {
	OPEN: 'open',
	OPEN_2: 'open2',
	CLOSE: 'close',
	MESSAGE_RECEIVED: 'message-received',
	MESSAGE_RECEIVED_2: 'message-received2',
	MESSAGE_SENT: 'message-sent',
	ERROR: 'error'
};

Dash.Socket.State = {
	CONNECTING: 0,
	OPEN: 1,
	CLOSING: 2,
	CLOSED: 3
};

Dash.Socket.prototype.open = function(host, port, socketId) {
	var self = this;

	this.host = host;
	this.port = port;

	this.ws = new WebSocket('ws://' + this.host + ':' + this.port);
	this.opening = true;
	
	this.ws.onopen = function() {
		this.opening = false;
		if(socketId == dash.config.socket.socketId) {
			self.fire({
				type: Dash.Socket.Event.OPEN,
				socket: self
			});
		} else if(socketId == dash.config.socket2.socketId) {
			self.fire({
				type: Dash.Socket.Event.OPEN_2,
				socket2: self
			});
		} else {
			alert('unknown socketId on OPEN');
		}
	};


	this.ws.onclose = function() {
		self.fire({
			type: Dash.Socket.Event.CLOSE,
			socket: self
		});
	};
	
	this.ws.onerror = function(message) {
		self.fire({
			type: Dash.Socket.Event.ERROR,
			message: message,
			socket: self
		});
	};

	this.ws.onmessage = function(message) {
		if(socketId == dash.config.socket.socketId) {
			self.fire({
				type: Dash.Socket.Event.MESSAGE_RECEIVED,
				message: message
			});
		} else if(socketId == dash.config.socket2.socketId) {
			self.fire({
				type: Dash.Socket.Event.MESSAGE_RECEIVED_2,
				message: message
			});
		} else {
			alert('unknown socketId on MESSAGE_RECEIVED');
		}


	};
};

Dash.Socket.prototype.getState = function() {
	if (this.ws == null) {
		return Dash.Socket.State.CLOSED;
	} else {
		return this.ws.readyState;
	}
};

Dash.Socket.prototype.isOpen = function() {
	return this.getState() == Dash.Socket.State.OPEN;
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
		
		if (state != Dash.Socket.State.OPEN) {
			this.fire({
				type: Dash.Socket.Event.ERROR,
				message: 'Unable to send message "' + message.replace('<', '&lt;').replace('>', '&gt;') + '", socket is in invalid state #' + state,
				socket: this
			});
			
			return;
		}
		this.ws.send(message);

		this.fire({
			type: Dash.Socket.Event.MESSAGE_SENT,
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