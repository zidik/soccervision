Dash.FrameCanvas = function(id) {
	this.id = id || 'frame-canvas';
	this.element = null;
	this.c = null;
};

Dash.FrameCanvas.prototype.init = function() {
	var self = this;
	
	this.element = document.getElementById(this.id);
	this.c = this.element.getContext('2d');
	
	$(this.element).mousemove(function(e) {
		if ($('#threshold-class').val() !== '') {
			self.render(e.offsetX, e.offsetY);
		} else {
			self.render(-1000, -1000);
		}
	});
	
	$(this.element).mouseleave(function(e) {
		self.render(-1000, -1000);
	});
	
	$(this.element).bind('mousewheel', function(e) {
		if ($('#threshold-class').val() === '') {
			return;
		}

		var currentIndex = $('#threshold-brush OPTION:selected').index(),
			newIndex = Math.min(Math.max(currentIndex + (e.originalEvent.wheelDelta > 0 ? 1 : -1), 0), $('#threshold-brush OPTION').length - 1);
		
		$('#threshold-brush OPTION:selected').removeAttr('selected');
		$('#threshold-brush OPTION:eq(' + newIndex + ')').attr('selected', 'selected');

		self.render(e.originalEvent.offsetX, e.originalEvent.offsetY);

		e.preventDefault();
	});
};

Dash.FrameCanvas.prototype.render = function(x, y) {
	var brush = parseInt($('#threshold-brush').val());
	
	this.element.width = this.width = $('#' + this.id).width();
	this.element.height = this.height = $('#' + this.id).height();
	
	this.c.clearRect(0, 0, this.width, this.height);
	
	this.c.strokeStyle = '#090';
	
	this.c.beginPath();
	this.c.arc(x, y, brush, 0 , 2 * Math.PI, false);
	this.c.closePath();
	this.c.stroke();
};