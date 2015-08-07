// Define the namespace
window.Dash = {};

Dash.Config = {
	socket: {
		host: 'localhost',
		port: 8000,
		socketId: 1
	},
	socket2: {
		host: '172.17.35.231',
		port: 8000,
		socketId: 2
	},
	robot: {
		radius: 0.125
	},
	robot2: {
		radius: 0.125
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
	}
};