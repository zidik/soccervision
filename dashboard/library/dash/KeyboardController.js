Dash.KeyboardController = function(robot) {
	var self = this;
	
	this.robot = robot;
	this.usedKeys = [87, 65, 83, 68, 69, 81, 16];
	this.enabled = false;
	this.updateInterval = window.setInterval(function() {
		self.updateRobotDir();
	}, 100);
};

Dash.KeyboardController.prototype.onKeyDown = function(key) {
	if (key == 32) {
		this.robot.kick();
	}
	
	if (this.usedKeys.indexOf(key) !== -1) {
		this.updateRobotDir();
	}
};

Dash.KeyboardController.prototype.onKeyUp = function(key) {
	if (this.usedKeys.indexOf(key) !== -1) {
		this.updateRobotDir();
	}
};

Dash.KeyboardController.prototype.updateRobotDir = function() {
	if (!this.enabled) {
		return;
	}
	
	var forwardDown = dash.ui.isKeyDown(87),
		leftDown = dash.ui.isKeyDown(65),
		reverseDown = dash.ui.isKeyDown(83),
		rightDown = dash.ui.isKeyDown(68),
		turnRightDown = dash.ui.isKeyDown(69),
		turnLeftDown = dash.ui.isKeyDown(81),
		shiftDown = dash.ui.isKeyDown(16),
		speed = dash.config.keyboard.speed,
		turnRate = dash.config.keyboard.turnRate,
		x = 0,
		y = 0,
		omega = 0;
	
	if (shiftDown) {
		speed *= 2;
		turnRate *= 2;
	}
	
	if (forwardDown) {
		x = speed;
	} else if (reverseDown) {
		x = -speed;
	}
	
	if (rightDown) {
		y = speed;
	} else if (leftDown) {
		y = -speed;
	}
	
	if (turnRightDown) {
		omega = turnRate;
	} else if (turnLeftDown) {
		omega = -turnRate;
	}
	
	this.robot.setTargetDir(x, y, omega);
};