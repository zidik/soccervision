Dash.WheelGraph = function(id) {
	this.id = id;
	this.element = null;
	this.c = null;
};

Dash.WheelGraph.prototype.init = function() {
	var self = this;
	
	this.element = document.getElementById(this.id);
	this.c = this.element.getContext('2d');
	this.element.width = this.width = $('#' + this.id).width();
	this.element.height = this.height = $('#' + this.id).height();
	this.lastState = null;
	this.lastName = null;
	
	$('#' + this.id).bind('resize', function() {
		self.element.width = self.width = $(this).width();
		self.element.height = self.height = $(this).height();
		
		if (self.lastState != null) {
			self.render(self.lastState, self.lastName);
		}
	});
};

Dash.WheelGraph.prototype.render = function(state, name) {
	this.c.clearRect(0, 0, this.width, this.height);
	
	if (state.robot[name].stalled) {
		this.c.fillStyle = '#900';
		this.c.fillRect(0, 0, this.width, this.height);
	}
	
	var x = this.width,
		currentState = state,
		ref = currentState.robot[name].ref || 0.007,
		drawLines = currentState.robot[name].drawLines ? true : false,
		multiplier = this.height * ref,
		range = 500,
		first = true,
		skipCount = 1,
		info,
		targetOmega,
		realOmega,
		y,
		i;
		
	this.c.lineWidth = 2;
	
	// draw target omega
	
	this.c.strokeStyle = '#333';

	if (drawLines) {
		for (var lineOmega = -range; lineOmega <= range; lineOmega += 10) {
			y = lineOmega * multiplier + this.height / 2;

			this.c.beginPath();
			this.c.moveTo(0, y);
			this.c.lineTo(this.width, y);
			this.c.stroke();
		}
	}
	
	this.c.strokeStyle = '#AA0';
	this.c.beginPath();
	
	while (currentState != null && x >= 0) {
		info = currentState.robot[name];
		//targetOmega = parseFloat(info.targetOmega);
		targetOmega = parseFloat(info.filteredTargetOmega);
		y = targetOmega * multiplier + this.height / 2;

		if (first) {
			this.c.moveTo(x, y);
			first = false;
		} else {
			this.c.lineTo(x, y);
		}
		
		x--;
		
		for (i = 0; i < skipCount; i++) {
			currentState = currentState.previous;
			
			if (currentState == null) {
				break;
			}
		}
	}
	
	this.c.stroke();
	
	// draw actual omega
	x = this.width;
	currentState = state;
		
	this.c.strokeStyle = '#0A0';
	this.c.beginPath();
	
	while (currentState != null && x >= 0) {
		info = currentState.robot[name];
		realOmega = parseFloat(info.realOmega);
		y = realOmega * multiplier + this.height / 2;

		if (first) {
			this.c.moveTo(x, y);
			first = false;
		} else {
			this.c.lineTo(x, y);
		}
		
		x--;
		
		for (i = 0; i < skipCount; i++) {
			currentState = currentState.previous;
			
			if (currentState == null) {
				break;
			}
		}
	}
	
	this.c.stroke();
	
	this.c.lineWidth = 1;
	this.c.fillStyle = '#FFF';
	this.c.beginPath();
	this.c.moveTo(0, this.height / 2 + -3);
	this.c.lineTo(6, this.height / 2);
	this.c.lineTo(0, this.height / 2 + 3);
	this.c.closePath();
	this.c.fill();
	
	this.c.beginPath();
	this.c.moveTo(this.width, this.height / 2 + -3);
	this.c.lineTo(this.width - 6, this.height / 2);
	this.c.lineTo(this.width, this.height / 2 + 3);
	this.c.closePath();
	this.c.fill();
	
	this.lastState = state;
	this.lastName = name;
};