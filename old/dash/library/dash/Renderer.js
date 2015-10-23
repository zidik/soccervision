Dash.Renderer = function(id) {
	this.id = id || 'canvas';
	this.element = null;
	this.c = null;
	this.renderDriveTo = false;
	this.driveToOrientation = 0.0;
	this.mouseX = 0;
	this.mouseY = 0;
	
	this.wheelGraphs = {
		FL: null,
		FR: null,
		RL: null,
		RR: null
	};
	this.voltageGraph = null;
};

Dash.Renderer.prototype.init = function() {
	this.element = document.getElementById(this.id);
	this.c = this.element.getContext('2d');
	
	this.widthToHeightRatio = 4.5 / 3.0;
	this.canvasWidth = 600;
	this.canvasHeight = this.canvasWidth / this.widthToHeightRatio;
	this.canvasToWorldRatio = this.canvasWidth / 4.5;
	this.fieldOffsetX = -(6.0 - 4.5) / 2;
	this.fieldOffsetY = -(4.0 - 3.0) / 2;
	
	this.c.setTransform(
		this.canvasToWorldRatio,
		0,
		0,
		this.canvasToWorldRatio,
		100,
		66
	);
		
	this.width = this.element.width;
	this.height = this.element.height;
	
	this.wheelGraphs = {
		FL: new Dash.WheelGraph('wheel-graph-fl'),
		FR: new Dash.WheelGraph('wheel-graph-fr'),
		RL: new Dash.WheelGraph('wheel-graph-rl'),
		RR: new Dash.WheelGraph('wheel-graph-rr')
	};

	this.voltageGraph = new Dash.WheelGraph('voltage-graph');
	this.voltageGraph.init();
	
	for (var name in this.wheelGraphs) {
		this.wheelGraphs[name].init();
	}

	$('#' + this.id).mousemove(function(e) {
		this.mouseX = e.offsetX;
		this.mouseY = e.offsetY;
	}.bind(this));

	$('#' + this.id).click(function(e) {
		if (this.renderDriveTo) {
			var mouseX = e.offsetX,
				mouseY = e.offsetY,
				pos = this.mouseToWorldCoords(mouseX, mouseY);

			dash.ui.driveTo(pos.x, pos.y, this.driveToOrientation);

			this.renderDriveTo = false;
		}
	}.bind(this));

	$('#' + this.id).mousewheel(function(event, delta, deltaX, deltaY) {
		if (!this.renderDriveTo) {
			return;
		}

		this.driveToOrientation -= delta * Math.PI / 8.0;
	}.bind(this));
};

Dash.Renderer.prototype.mouseToWorldCoords = function(mouseX, mouseY) {
	return {
		x: mouseX / this.canvasToWorldRatio + this.fieldOffsetX,
		y: mouseY / this.canvasToWorldRatio + this.fieldOffsetY
	}
};

Dash.Renderer.prototype.showDriveTo = function() {
	this.renderDriveTo = true;
	this.driveToOrientation = 0.0;
};

Dash.Renderer.prototype.drawRobot = function(radius, color, x, y, orientation, gotBall) {
	this.c.save();
	
	this.c.translate(x, y);
	this.c.rotate(orientation);
	this.c.fillStyle = color;
	this.c.beginPath();
	this.c.arc(0, 0, radius, 0, Math.PI * 2, true); 
	this.c.closePath();
	this.c.fill();

	var dirHeight = radius / 5;

	this.c.fillStyle = '#FFF';
	this.c.fillRect(0, -dirHeight / 2.0, radius, dirHeight);

	if (gotBall) {
		//this.c.fillStyle = '#FFA500';
		this.c.fillStyle = '#FF0000';
		this.c.beginPath();
		this.c.arc(radius, 0, dash.config.ball.radius * 2, 0, Math.PI * 2, true);
		this.c.closePath();
		this.c.fill();
	}
	
	this.c.restore();
};

Dash.Renderer.prototype.drawBalls = function(balls, color, radius, useEffects) {
	for (var i = 0; i < balls.length; i++) {
		this.drawBall(balls[i], color, radius, useEffects);
	}
};

Dash.Renderer.prototype.drawBall = function(ball, color, radius, useEffects) {
	this.c.save();

	this.c.translate(ball.x, ball.y);

	if (!radius) {
		radius = dash.config.ball.radius;
	}

	if (ball.visible) {
		radius *= 2;
	}

	if (ball.inFOV && useEffects) {
		this.c.fillStyle = '#F00';
		this.c.beginPath();
		this.c.arc(0, 0, radius * 1.25, 0, Math.PI * 2, true);
		this.c.closePath();
		this.c.fill();
	}

	this.c.fillStyle = color;
	this.c.beginPath();
	this.c.arc(0, 0, radius, 0, Math.PI * 2, true);
	this.c.closePath();
	this.c.fill();

	this.c.restore();
};

Dash.Renderer.prototype.drawPolygon = function(points, color) {
	if (points.length === 0) {
		return;
	}

	this.c.save();

	this.c.fillStyle = color;
	this.c.beginPath();
	this.c.moveTo(points[0].x, points[0].y);

	for (var i = 1; i < points.length; i++) {
		this.c.lineTo(points[i].x, points[i].y);
	}

	this.c.closePath();
	this.c.fill();

	this.c.restore();
};

Dash.Renderer.prototype.drawIntersections = function(yellowDistance, blueDistance) {
	this.c.save();

	this.c.lineWidth = 1 / this.canvasToWorldRatio;

	if (yellowDistance > 0) {
		this.c.strokeStyle = '#DD0';
		this.c.beginPath();
		this.c.arc(0, Dash.Config.field.height / 2, yellowDistance, 0, Math.PI * 2, true);
		this.c.closePath();
		this.c.stroke();
	}

	if (blueDistance > 0) {
		this.c.strokeStyle = '#00F';
		this.c.beginPath();
		this.c.arc(Dash.Config.field.width, Dash.Config.field.height / 2, blueDistance, 0, Math.PI * 2, true);
		this.c.closePath();
		this.c.stroke();
	}

	this.c.restore();
};

Dash.Renderer.prototype.drawParticle = function(x, y) {
	this.c.save();

	this.c.fillStyle = '#F00';
	this.c.fillRect(x, y, 0.01, 0.01);

	this.c.restore();
};

Dash.Renderer.prototype.drawRuler = function() {
	this.c.save();
	
	this.c.beginPath();
	this.c.lineWidth = 0.3;
	this.c.strokeStyle = '#090';
	this.c.moveTo(1.0, 1.0);
	this.c.lineTo(3.0, 1.0);
	this.c.closePath();
	this.c.stroke();
	
	this.c.restore();
};

Dash.Renderer.prototype.drawPath = function(state/*, localizer*/, color, items) {
	items = typeof(items) !== 'undefined' ? items : 60 * 5;

	/*if (state.controllerState === null) {
		return;
	}*/

	//var data = state.controllerState[localizer],
	var previousState = state.previous,
		skips = 10;

	if (!previousState) {
		return;
	}

	/*for (var i = 0; i < skips; i++) {
		if (!previousState.previous) {
			return;
		}

		previousState = previousState.previous;
	}*/

	if (arguments.length == 4) {
		var last = arguments[3];

		this.drawPathSegment(last.robot.localizerX, last.robot.localizerY, state.robot.localizerX, state.robot.localizerY, color);
	}

	if (items > 0) {
		this.drawPath(previousState, color, items - 1, state);
	}
};

Dash.Renderer.prototype.drawPathSegment = function(x1, y1, x2, y2, color) {
	this.c.save();

	this.c.beginPath();
	this.c.lineWidth = 1.0 / this.canvasToWorldRatio;
	this.c.strokeStyle = color;
	this.c.moveTo(x1, y1);
	this.c.lineTo(x2, y2);
	this.c.stroke();

	this.c.restore();
};

Dash.Renderer.prototype.drawMarkers = function() {
	var hor = 1.3,
		ver = 0.8;

	this.drawMarker(hor, ver);
	this.drawMarker(dash.config.field.width - hor, ver);
	this.drawMarker(hor, dash.config.field.height - ver);
	this.drawMarker(dash.config.field.width - hor, dash.config.field.height - ver);
};

Dash.Renderer.prototype.drawMarker = function(x, y) {
	this.c.save();

	this.c.translate(x, y);
	this.c.fillStyle = '#F00';
	this.c.beginPath();
	this.c.arc(0, 0, 0.05, 0, Math.PI * 2, true);
	this.c.closePath();
	this.c.fill();

	this.c.restore();
};

Dash.Renderer.prototype.drawDriveTo = function(x, y) {
	var pos = this.mouseToWorldCoords(this.mouseX, this.mouseY);

	this.drawRobot(
		dash.config.robot.radius,
		'rgba(255, 0, 0, 0.5)',
		pos.x,
		pos.y,
		this.driveToOrientation
	);
};

Dash.Renderer.prototype.renderState = function(state) {
	this.c.clearRect(-1, -1, this.width + 1, this.height + 1);
	
	//this.drawRuler();
	
	this.drawRobot(
		dash.config.robot.radius,
		state.targetSide == 1 ? '#00F' : state.targetSide == 0 ? '#DD0' : '#CCC',
		state.robot.localizerX,
		state.robot.localizerY,
		state.robot.localizerOrientation,
		state.robot.gotBall
	);

	this.drawRobot(
		dash.config.robot.radius / 2,
		'#900',
		state.robot.odometerX,
		state.robot.odometerY,
		state.robot.odometerOrientation
	);

	this.drawPath(state, '#060');

	this.drawPolygon(state.robot.cameraFOV, 'rgba(255, 255, 255, 0.25)');

	this.drawBalls(
		state.robot.ballsRaw,
		'#006',
		dash.config.ball.radius * 2,
		false
	);

	this.drawBalls(
		state.robot.ballsFiltered,
		'#FFA500',
		dash.config.ball.radius,
		true
	);

	/*this.drawRobot(
		dash.config.robot.radius,
		'#CCC',
		-0.4,
		0.4,
		state.gyroOrientation
	);*/

	if (state.controllerState !== null) {
		/*if (state.controllerState.odometerLocalizer !== null && typeof(state.controllerState.odometerLocalizer) === 'object') {
			this.drawRobot(
				dash.config.robot.radius / 2,
				'#600',
				state.controllerState.odometerLocalizer.x,
				state.controllerState.odometerLocalizer.y,
				state.controllerState.odometerLocalizer.orientation
			);

			this.drawPath(state, 'odometerLocalizer', '#600');
		}*/

		/*if (state.controllerState.intersectionLocalizer !== null && typeof(state.controllerState.intersectionLocalizer) === 'object') {
			this.drawIntersections(
				state.controllerState.intersectionLocalizer.yellowDistance,
				state.controllerState.intersectionLocalizer.blueDistance
			);

			this.drawRobot(
				dash.config.robot.radius / 2,
				'#660',
				state.controllerState.intersectionLocalizer.x,
				state.controllerState.intersectionLocalizer.y,
				state.controllerState.intersectionLocalizer.orientation
			);

			this.drawPath(state, 'intersectionLocalizer', '#660');
		}*/

		/*if (state.controllerState.kalmanLocalizer !== null && typeof(state.controllerState.kalmanLocalizer) === 'object') {
			this.drawRobot(
				dash.config.robot.radius,
				'#006',
				parseFloat(state.controllerState.kalmanLocalizer.x),
				parseFloat(state.controllerState.kalmanLocalizer.y),
					parseFloat(state.controllerState.kalmanLocalizer.orientation)
			);

			this.drawPath(state, 'kalmanLocalizer', '#006');
		}*/

		if (state.controllerState.particleLocalizer !== null && typeof(state.controllerState.particleLocalizer) === 'object') {
			this.drawRobot(
				dash.config.robot.radius,
				'#060',
				state.controllerState.particleLocalizer.x,
				state.controllerState.particleLocalizer.y,
				state.controllerState.particleLocalizer.orientation
			);

			for (var i = 0; i < state.controllerState.particleLocalizer.particles.length; i++) {
				this.drawParticle(
					state.controllerState.particleLocalizer.particles[i][0],
					state.controllerState.particleLocalizer.particles[i][1]
				);
			}

			//this.drawPath(state, 'particleLocalizer', '#060');
		}

		if (state.controllerState.blueGoalDistance || state.controllerState.yellowGoalDistance) {
			this.drawIntersections(
				state.controllerState.yellowGoalDistance,
				state.controllerState.blueGoalDistance
			);
		}
	}

	//this.drawMarkers();

	if (this.renderDriveTo) {
		this.drawDriveTo();
	}
		
	this.wheelGraphs.FL.render.apply(this.wheelGraphs.FL, [state, 'wheelFL']);
	this.wheelGraphs.FR.render.apply(this.wheelGraphs.FR, [state, 'wheelFR']);
	this.wheelGraphs.RL.render.apply(this.wheelGraphs.RL, [state, 'wheelRL']);
	this.wheelGraphs.RR.render.apply(this.wheelGraphs.RR, [state, 'wheelRR']);

	var graphVoltage = -(state.robot.coilgun.voltage - 200);

	// fake the voltage reading to be a wheel..
	state.robot.coilGraph = {
		stalled: state.robot.coilgun.isLowVoltage,
		targetOmega: graphVoltage,
		filteredTargetOmega: graphVoltage,
		realOmega: graphVoltage,
		ref: 0.0025,
		drawLines: false
	};

	this.voltageGraph.render.apply(this.voltageGraph, [state, 'coilGraph']);
};