Dash.BlobberView = function(id) {
	this.id = id || 'blobber-canvas';
	this.element = null;
	this.c = null;
};

Dash.BlobberView.prototype.init = function() {
	this.element = document.getElementById(this.id);
	this.c = this.element.getContext('2d');
	this.element.width = this.width = $('#' + this.id).width();
	this.element.height = this.height = $('#' + this.id).height();
};

Dash.BlobberView.prototype.render = function(state) {
	this.c.clearRect(0, 0, this.width, this.height);
	
	this.c.strokeStyle = '#000';
	this.c.beginPath();
	this.c.moveTo(this.width / 2, 0);
	this.c.lineTo(this.width / 2, this.height);
	this.c.stroke();
	
	this.c.strokeStyle = '#F00';
	
	var ball, goal, dx;
	
	for (var i = 0; i < state.balls.length; i++) {
		ball = state.balls[i];
		dx = ball.camera == 'front' ? 0 : 640;
		
		this.c.strokeRect(ball.x - ball.width / 2 + dx, ball.y - ball.height / 2, ball.width, ball.height);
	}
};