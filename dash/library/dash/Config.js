// Define the namespace
window.Dash = {};

Dash.Config = function() {
	this.socket = null;
	this.robot = null;
	this.controls = null;
	this.ball = {
		radius: 0.021335
	};
	this.field = {
		width: 4.5,
		height: 3.0
	};
	this.keyboard = {
		speed: 0.5,
		turnRate: Math.PI
	};
	this.joystick = {
		speed: 1.5,
		turnRate: Math.PI * 2
	};
};


Dash.Config.prototype.init = function(robotNr) {
	this.socket = {
		host: 'localhost',
		port: 8000,
		socketId: 'soc' + robotNr,
		Events: {
			OPEN: 'open' + robotNr,
			CLOSE: 'close' + robotNr,
			MESSAGE_RECEIVED: 'message-received' + robotNr,
			MESSAGE_SENT: 'message-sent' + robotNr,
			ERROR: 'error' + robotNr
		}

	};/*
	socket2: {
		host: '172.17.35.231',
		port: 8000,
		socketId: 'soc2',
		Events: {
			OPEN: 'open2',
			CLOSE: 'close2',
			MESSAGE_RECEIVED: 'message-received2',
			MESSAGE_SENT: 'message-sent2',
			ERROR: 'error2'
		},
		States: {
			CONNECTING: 0,
			OPEN: 1,
			CLOSING: 2,
			CLOSED: 3
		}
	},*/
	this.robot = {
		radius: 0.125 + robotNr,
		robotId: 'samott' + robotNr,
		robotIndex: robotNr
	};/*
	robot2: {
		radius: 0.125,
		robotId: 'samott2'
	},*/



	this.controls = {
		hostBtn:{
			btnName: 'Host ' + robotNr,
			btnId: 'host-btn' + robotNr
		},
		controllerChoice: {
			btnName: 'STOP ' + robotNr,
			btnId: 'stop-btn' + robotNr
		}
	};
	/*controls2: {
		hostBtn: '#host-btn2',
		controllerChoice: '#controller-choice'
	}*/
};
