// Define the namespace
window.Dash = {};

Dash.Config = {
	socket: {
		host: 'localhost',
		port: 8000,
		socketId: 'soc1',
		Events: {
			OPEN: 'open',
			CLOSE: 'close',
			MESSAGE_RECEIVED: 'message-received',
			MESSAGE_SENT: 'message-sent',
			ERROR: 'error'
		},
		States: {
		CONNECTING: 0,
		OPEN: 1,
		CLOSING: 2,
		CLOSED: 3
		}
	},
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
	},
	robot: {
		radius: 0.125,
		robotId: 'samott1'
	},
	robot2: {
		radius: 0.125,
		robotId: 'samott2'
	},
	ball: {
		radius: 0.021335
	},
	field: {
		width: 4.5,
		height: 3.0
	},
	keyboard: {
		speed: 0.5,
		turnRate: Math.PI
	},
	joystick: {
		speed: 1.5,
		turnRate: Math.PI * 2
	},
	controls: {
		hostBtn: '#host-btn',
		controllerChoice: '#controller-choice'
	},
	controls2: {
		hostBtn: '#host-btn2',
		controllerChoice: '#controller-choice'
	}
};
