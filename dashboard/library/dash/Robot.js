Dash.Robot = function(socket) {
	this.socket = socket;
	this.controller = null;
	this.dribblerActive = false;
};

Dash.Robot.prototype.setController = function(name) {
	if (this.socket.isOpen() && name != this.controller) {
		dash.dbg.log('! Setting controller: ' + name);
		
		this.socket.send('<set-controller:' + name + '>');
		
		this.controller = name;
	}
};

Dash.Robot.prototype.kick = function(duration) {
	duration = duration || 1500;
	//duration = duration || 3250;
	//duration = duration || 6500;

	if (this.socket.isOpen()) {
		dash.dbg.log('! Kicking for duration: ' + duration + 'usec');
		
		//this.socket.send('<kick:5000>');
		this.socket.send('<kick:' + duration + '>');
	}
};

Dash.Robot.prototype.chipKick = function(distance) {
	distance = distance || 1.0;

	if (this.socket.isOpen()) {
		dash.dbg.log('! Chip-kicking to distance: ' + distance + 'm');

		//this.socket.send('<kick:5000>');
		this.socket.send('<chip-kick:' + distance + '>');
	}
};

Dash.Robot.prototype.toggleDribbler = function() {
	if (this.socket.isOpen()) {
		dash.dbg.log('! Toggling dribbler');
		
		this.dribblerActive = !this.dribblerActive;
		
		if (this.dribblerActive) {
			this.socket.send('<set-dribbler:100>');
		} else {
			this.socket.send('<set-dribbler:0>');
		}
	}
};

Dash.Robot.prototype.adjustDribbler = function(lowerLimitDelta, upperLimitDelta) {
	if (this.socket.isOpen()) {
		dash.dbg.log('! Adjusting dribbler limits', lowerLimitDelta, upperLimitDelta);

		this.socket.send('<adjust-dribbler-limits:' + lowerLimitDelta + ':' + upperLimitDelta + '>');
	}
};

Dash.Robot.prototype.useNormalDribblerLimits = function() {
	if (this.socket.isOpen()) {
		this.socket.send('<dribbler-normal-limits>');
	}
};

Dash.Robot.prototype.useChipKickDribblerLimits = function() {
	if (this.socket.isOpen()) {
		this.socket.send('<dribbler-chip-kick-limits>');
	}
};

Dash.Robot.prototype.setDribbler = function(speed) {
	if (this.socket.isOpen()) {
		dash.dbg.log('! Setting dribbler speed: ' + speed);
		
		this.socket.send('<set-dribbler:' + speed + '>');
	}
};

Dash.Robot.prototype.setTargetDir = function(x, y, omega) {
	if (this.controller != 'manual' && this.controller != 'test') {
		return;
	}
	
	//dash.dbg.log('! Robot target', x, y, omega);
	
	if (this.socket.isOpen()) {
		this.socket.send('<target-vector:' + x + ':' + y + ':' + omega + '>');
	}
};

Dash.Robot.prototype.resetPosition = function() {
	dash.dbg.log('! Resetting position');
	
	dash.socket.send('<reset-position>');
};

Dash.Robot.prototype.turnBy = function(angle, speed) {
	speed = typeof(speed) == 'number' ? speed : 1.0;
	
	dash.dbg.log('! Turning by ' + Dash.Util.radToDeg(angle) + '°');
	
	dash.socket.send('<test-turn-by:' + angle + ':' + speed + '>');
};

Dash.Robot.prototype.driveTo = function(x, y, orientation, speed) {
	speed = typeof(speed) == 'number' ? speed : 1.0;
	
	dash.dbg.log('! Driving to ' + x + 'x' + y + ' @ ' + Dash.Util.radToDeg(orientation) + '°');
	
	dash.socket.send('<test-drive-to:' + x + ':' + y + ':' + orientation + ':' + speed + '>');
};

Dash.Robot.prototype.driveFacing = function(targetX, targetY, faceX, faceY, speed) {
	speed = typeof(speed) == 'number' ? speed : 1.0;
	
	dash.dbg.log('! Driving to ' + targetX + 'x' + targetY + ' facing ' + faceX + 'x' + faceY);
	
	dash.socket.send('<test-drive-facing:' + targetX + ':' + targetY + ':' + faceX + ':' + faceY + ':' + speed + '>');
};

Dash.Robot.prototype.testRectangle = function() {
	dash.dbg.log('! Running rectangle test');
	
	dash.socket.send('<test-rectangle>');
};

// dash.ui.robot.setCameraTranslatorConstants(108.81935671519336, -0.025382067623573618, 0.20689130201672276, -2.829515376853199426e-01, 7.099480368253494045e-02, -7.076742557435421188e-03, 163.41286, 6.904681785333543758e+02);
Dash.Robot.prototype.setCameraTranslatorConstants = function(A, B, C, k1, k2, k3, horizon, distortionFocus) {
	dash.dbg.log('! Sending camera translator constants', A, B, C, k1, k2, k3, horizon, distortionFocus);

	dash.socket.send('<camera-translator:' + A + ':' + B + ':' + C + ':' + k1 + ':' + k2 + ':' + k3 + ':' + horizon + ':' + distortionFocus + '>');
};