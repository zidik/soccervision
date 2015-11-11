Dash.FpsCounter = function(changeCallback, interval) {
	this.changeCallback = changeCallback || null;
	this.interval = interval || 60;
	this.startTime = null;
	this.frames = 0;
	this.previousFPS = 0;
	this.lastFPS = 0;
	
	if (typeof(this.changeCallback) == 'function') {
		var self = this;
		
		window.setInterval(function() {
			if (self.lastFPS != self.previousFPS) {
				self.changeCallback(self.lastFPS, self.previousFPS);
				
				self.previousFPS = self.lastFPS;
			}
		}, 500);
	}
};

Dash.FpsCounter.prototype.step = function() {
	if (this.startTime == null) {
		this.startTime = Dash.Util.getMicrotime();
		this.frames = 1;
		
		return;
	}
	
	if (this.frames >= this.interval) {
		var currentTime = Dash.Util.getMicrotime(),
			elapsedTime = currentTime - this.startTime;
		
		this.lastFPS = this.frames / elapsedTime;
		this.startTime = currentTime;
		this.frames = 0;
	} else {
		this.frames++;
	}
};

Dash.FpsCounter.prototype.getLastFPS = function() {
	return this.lastFPS;
};