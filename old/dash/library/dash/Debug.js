/**
 * Debugging helper.
 *
 * Can fire the following events:
 * > ERROR - fired when a user error occurs
 *   message - error message
 * > CONSOLE - fired when user send something to display in console
 *   args - console arguments
 */
Dash.Debug = function() {
	this.messages = [];
	this.boxes = {};
	this.boxCount = 0;
};

/**
 * Extend the EventTarget for custom event handling.
 */
Dash.Debug.prototype = new Dash.Bindable();

/**
 * Debugger event types.
 */
Dash.Debug.Event = {
	ERROR: 'error',
	CONSOLE: 'console',
	LOG: 'log',
	EXTERNAL: 'external'
};

/**
 * Initiates the debugger.
 */
Dash.Debug.prototype.init = function() {
	var self = this;
	
	window.setInterval(function() {
		self.step();
	}, 100);
};

/**
 * Triggers a error message.
 *
 * The message is passed on to any listeners of given type.
 *
 * @param {string} message Message
 */
Dash.Debug.prototype.error = function(message) {
	this.fire({
		type: Dash.Debug.Event.ERROR,
		message: message
	});
};

/**
 * Logs data to console if available.
 *
 * @param {any} ... Variable number of arguments
 */
Dash.Debug.prototype.console = function() {
	this.fire({
		type: Dash.Debug.Event.CONSOLE,
		args: arguments
	});
};

/**
 * Logs data to screen.
 *
 * @param {any} ... Variable number of arguments
 */
Dash.Debug.prototype.log = function() {
	this.fire({
		type: Dash.Debug.Event.LOG,
		args: arguments
	});
};

/**
 * Logs external data to screen.
 *
 * @param {any} ... Variable number of arguments
 */
Dash.Debug.prototype.external = function() {
	this.fire({
		type: Dash.Debug.Event.EXTERNAL,
		args: arguments
	});
};

/**
 * Creates a debug-box.
 * 
 * @param {string} name Name of the item
 * @param {mixed} value Value
 * @param {integer} [numberDecimals] Number of decimals for float value
 */
Dash.Debug.prototype.box = function(name, value, numberDecimals) {
	if (this.boxes[name] == null) {
		var container = $('#debug-wrap');
		
		if (container.length == 0) {
			$(document.body).append($('<ul/>', {
				'id': 'debug-wrap'
			}));
			
			container = $('#debug-wrap')
		}
		
		var boxId = 'debug-box-' + this.boxCount;
		
		container.append($('<li/>', {
			'id': boxId,
			'class': 'debug-value'
		}));
		
		var boxElement = $('#' + boxId);
		
		this.boxes[name] = {
			id: boxId,
			element: boxElement,
			firstValue: value,
			lastValue: value,
			lastUpdated: Dash.Util.getMicrotime()
		};
		
		this.boxCount++;
	}
	
	var box = this.boxes[name],
		element = box.element,
		displayValue = value;
	
	box.lastUpdated = Dash.Util.getMicrotime();
	
	if (typeof(value) == 'number' && typeof(numberDecimals) == 'number') {
		displayValue = Dash.Util.round(value, numberDecimals);
	}
	
	element.html('<strong>' + name + '</strong>' + (typeof(displayValue) != 'undefined' ? ': <span>' + displayValue + '</span>' : ''));
	
	box.lastValue = value;
};

/**
 * Updates the debugger.
 */
Dash.Debug.prototype.step = function() {
	for (var name in this.boxes) {
		if (Dash.Util.getMicrotime() - this.boxes[name].lastUpdated > 1.0) {
			$(this.boxes[name].element).remove();
			
			delete this.boxes[name];
		}
	}
};