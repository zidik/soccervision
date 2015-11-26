Dash.UI = function() {
	this.states = [];
	this.keystates = {};
	this.reconnectTimeout = null;
	this.stateSlider = null;
	this.currentStateIndexWrap = null;
	this.stateCountWrap = null;
	this.keyboardController = null;
	this.joystickController = null;
	this.lastLogMessage = null;
	this.rxCounter = null;
	this.blobberView = null;
	this.frameCanvas = null;
	this.currentStateIndex = 0;
	this.repeatedLogCount = 0;
	this.extractedCameraTranslator = false;

};

Dash.UI.prototype = new Dash.Bindable();

Dash.UI.Event = {
	KEY_DOWN: 'key-down',
	KEY_UP: 'key-up'
};

Dash.UI.prototype.init = function(socket, config) {
	this.socket = socket;
    this.config = config;
    this.robotId = config.robot.robotId;
	this.robotIndex = config.robot.robotIndex;
	this.socketEvents = socket.socketEvents;
	this.initDebugListener();
	//this.initSlider();
	this.initSocket();
    this.initRobot();
	this.initFpsCounter();
	this.initKeyboardController();
	this.initJoystickController();
	this.initKeyListeners();
	this.initControls();
	this.initBlobberView();
	this.initFrameCanvas();
};

Dash.UI.prototype.initDebugListener = function() {
	dash.dbg.bind(Dash.Debug.Event.CONSOLE, function(event) {
		if (
			typeof(console) == 'object'
			&& typeof(console.log) == 'function'
		) {
			var time = Dash.Util.getTime();

			var args = [time];
			
			for (var i = 0; i < event.args.length; i++) {
				args.push(event.args[i]);
			}
			
			console.log.apply(console, args);
		}
	});
	
	dash.dbg.bind([Dash.Debug.Event.LOG, Dash.Debug.Event.EXTERNAL], function(event) {
		var wrap = $('#log'),
			msgClass = 'normal',
			message = event.args[0],
			firstChar = message.substr(0, 1);
			
		if (message == this.lastLogMessage) {
			this.repeatedLogCount++;
			
			message = message + ' (' + (this.repeatedLogCount + 1) + ')';
			
			wrap.find('DIV:last').remove();
		}else {
			this.lastLogMessage = message;
			this.repeatedLogCount = 0;
		}

		if (firstChar == '+') {
			msgClass = 'positive';
		} else if (firstChar == '-') {
			msgClass = 'negative';
		}else if (firstChar == '!') {
			msgClass = 'info';
		}

		if (wrap.find('DIV').length >= 1000) {
			wrap.html('');
		}

		var content = message,
			arg,
			args = [],
			i;

		for (i = 1; i < event.args.length; i++) {
			arg = event.args[i];

			if (typeof(arg) == 'string') {
				content += ', ' + arg;
			} else {
				content += ', ' + JSON.stringify(arg);
			}

			args.push(arg);
		}
		
		if (event.type == Dash.Debug.Event.EXTERNAL) {
			msgClass += ' external';
		}

		wrap.append(
			'<div class="' + msgClass + '">' + content + '</div>'
		);
			
		wrap.prop(
			'scrollTop',
			wrap.prop('scrollHeight')
		);
	});
	
	dash.dbg.bind(Dash.Debug.Event.ERROR, function(event) {
		if (
			typeof(console) == 'object'
			&& typeof(console.log) == 'function'
		) {
			var time = Dash.Util.getTime();

			var args = [time];
			
			for (var i = 0; i < event.args.length; i++) {
				args.push(event.args[i]);
			}
			
			console.error.apply(console, args);
		}
	});
	
	$('#log').mousedown(function(e) {
		if (e.which == 3) {
			$(this).empty();
		}
	});
	
	$('#log').bind('contextmenu', function(e) {
		if (e.which == 3) {
			return false;
		}
	});
	
	$('#log').hover(
		function() {
			$(this).stop(true, false).animate({
				width: '1200px'
			}, 300);
		},
		function() {
			$(this).stop(true, false).animate({
				width: '300px'
			}, 150);
		}
	);
};

Dash.UI.prototype.initSlider = function() {
	var self = this;

	this.stateSlider = $('#state-slider');
	this.currentStateIndexWrap = $('#current-state-index');
	this.stateCountWrap = $('#state-count');

	this.stateSlider.slider({
		onChange: function(value) {
			self.showState(value - 1);
		}
	});

	this.currentStateIndexWrap.bind('change keyup click mousewheel', function(e) {
		var newIndex = parseInt(self.currentStateIndexWrap.val()) - 1;

		if (newIndex > self.states.length - 1) {
			newIndex = self.states.length - 1;
		} else if (newIndex < 0) {
			newIndex = 0;
		}

		self.currentStateIndexWrap.html(newIndex + 1);

		self.showState(newIndex);
	});

	$('.slider').slider({
		showRange: true,
		showValue: true,
		width: 300,
		minChangeInterval: 500
	});
};

Dash.UI.prototype.setupParameterFields = function() {
    var self = this;
	$('.send-parameter-field').each(function() {
		var index = $(this).data('index'),
			value = parseFloat(window.localStorage['parameter-' + index]);

		if (isNaN(value)) {
			value = 0.0;
		}

		$(this).val(value);
        self.socket.send('<parameter:' + index + ':' + value + '>');
	});
};

Dash.UI.prototype.initSocket = function() {
	var self = this,
		cookieHost = $.cookie('host-' + self.robotId);

	if (cookieHost != null) {
		self.config.socket.host = cookieHost;
	}
	/*
    var eventOpen, eventMsgRcvd;
    if (this.robotId == dash.config.robot.robotId) {
        eventOpen = Dash.Socket.Event.OPEN;
        eventMsgRcvd = Dash.Socket.Event.MESSAGE_RECEIVED;
    } else if (this.robotId == dash.config2.robot.robotId) {
        eventOpen = Dash.Socket.Event.OPEN_2;
        eventMsgRcvd = Dash.Socket.Event.MESSAGE_RECEIVED_2;
    } else {
        alert("robotId not set on socket");
    }
	*/
	this.socket.bind(this.socketEvents.OPEN, function(e) {
		if (self.reconnectTimeout != null) {
			window.clearTimeout(self.reconnectTimeout);
			
			self.reconnectTimeout = null;
		}
		
		dash.dbg.log(
			'+ Socket server opened on ' + e.socket.host + ':' + e.socket.port
		);
			
		$('#connecting').hide();
		$('#buttons-'+self.robotId+' .live-only').removeAttr('disabled');
		$('#rebuild-btn').text('Rebuild');
		
		//window.setTimeout(function() {
        self.socket.send('<get-controller>');
		//}, 2000);
		self.socket.send('<set-dash-index:' + self.robotIndex + '>');
		//dash.commonLogic.connectedRobots++;

		self.setupParameterFields();
	});
	
	this.socket.bind(this.socketEvents.CLOSE, function(e) {
		dash.dbg.log('- Socket server closed');
		var rem_index = dash.commonLogic.connectedRobots.indexOf(self.robotIndex);
		if (rem_index > -1) {
			dash.commonLogic.connectedRobots.splice(rem_index, 1);
		}
		dash.commonLogic.disconnectedRobots.push(self.robotIndex);
		$('#connecting').show();
		$('#buttons-'+self.robotId+' .live-only').attr('disabled', 'disabled');
		
		if (self.reconnectTimeout != null) {
			window.clearTimeout(self.reconnectTimeout);
			
			self.reconnectTimeout = null;
		}
		
		self.reconnectTimeout = window.setTimeout(function() {
            self.socket.open(self.config.socket.host, self.config.socket.port);
		}, 1000);
		
		$('#controller-choice OPTION:eq(0)').trigger('select');
	});
	
	this.socket.bind(this.socketEvents.ERROR, function(e) {
		dash.dbg.log('- Socket error occured: ' + e.message);
	});

	this.socket.bind(this.socketEvents.MESSAGE_SENT, function(e) {
		self.flashClass('#tx', 'active', 100);
	});

	this.socket.bind(this.socketEvents.MESSAGE_RECEIVED, function(e) {

        var message;

        try {
            message = JSON.parse(e.message.data);
        } catch (ex) {
            dash.dbg.log('- Invalid message', e.message.data);
            debugger;

            return;
        }

            self.handleMessage(message,self.robotId);

            self.flashClass('#rx', 'active', 100);


	});
	//if(self.config.socket.host != 'localhost') {
	self.socket.open(self.config.socket.host, self.config.socket.port); //commented out for debugging
	//}
	/*window.setInterval(function() {
		if (socket.getState() != Dash.Socket.State.OPEN) {
			$('#connecting').show();
			
			socket.open(dash.config.socket.host, dash.config.socket.port);
		} else {
			$('#connecting').hide();
		}
	}, 1000);*/
};

Dash.UI.prototype.initRobot = function() {
	this.robot = new Dash.Robot(this.socket, this.robotId);
};

Dash.UI.prototype.initFpsCounter = function() {
	this.rxCounter = new Dash.FpsCounter(function(fps) {
		var wrap = $('#connection'),
			maxFps = 60.0,
			current = fps / maxFps;
		
		wrap.find('LI:eq(0)').attr('class', current >= 0.2 ? 'active' : '');
		wrap.find('LI:eq(1)').attr('class', current >= 0.4 ? 'active' : '');
		wrap.find('LI:eq(2)').attr('class', current >= 0.6 ? 'active' : '');
		wrap.find('LI:eq(3)').attr('class', current >= 0.8 ? 'active' : '');
		wrap.find('LI:eq(4)').attr('class', current >= 0.95 ? 'active' : '');
	});
};

Dash.UI.prototype.initKeyboardController = function() {
	this.keyboardController = new Dash.KeyboardController(this.robot);
};

Dash.UI.prototype.initJoystickController = function() {
	this.joystickController = new Dash.JoystickController(this.robot, this.socket);

	this.joystickController.gamepad.bind(Gamepad.Event.CONNECTED, function(device) {
		dash.dbg.log('! Controller connected', device);

		/*$('INPUT[name="joystick-controller-enabled"]')
			.removeAttr('disabled')
			.iphoneStyle('refresh');*/

		$('#gamepad').html(device.id);
	});

	this.joystickController.gamepad.bind(Gamepad.Event.DISCONNECTED, function(device) {
		dash.dbg.log('! Controller disconnected', device);
		
		/*$('INPUT[name="joystick-controller-enabled"]')
			.removeAttr('checked')
			.iphoneStyle('refresh')
			.attr('disabled', 'disabled')
			.iphoneStyle('refresh');*/
		
		$('#gamepad').html('Gamepad disconnected');
		
		self.useGamepad = false;
	});

	this.joystickController.gamepad.bind(Gamepad.Event.UNSUPPORTED, function(device) {
		$('#gamepad').html('Unsupported controller connected');
		
		dash.dbg.log('- Unsupported controller connected', device);
	});
	
	this.joystickController.init();
};

Dash.UI.prototype.initKeyListeners = function() {
	var self = this;
	
	$(document.body).keydown(function(e) {
		if (typeof(self.keystates[e.keyCode]) == 'undefined' || self.keystates[e.keyCode] == false) {
			self.keystates[e.keyCode] = true;
			
			self.onKeyDown(e);
		}
		
		self.fire({
			type: Dash.UI.Event.KEY_DOWN,
			key: e.keyCode,
			event: e
		});
	});
	
	$(document.body).keyup(function(e) {
		if (typeof(self.keystates[e.keyCode]) == 'undefined' || self.keystates[e.keyCode] == true) {
			self.keystates[e.keyCode] = false;
			
			self.onKeyUp(e);
		}
		
		self.fire({
			type: Dash.UI.Event.KEY_UP,
			key: e.keyCode,
			event: e
		});
	});
	
	$(window).blur(function() {
		for (var key in self.keystates) {
			if (self.keystates[key] == true) {
				self.onKeyUp(key);
			}
		}
		
		self.keystates = {};
	});
};

Dash.UI.prototype.initControls = function(controls) {

	$('<div/>', {
		id: 'buttons-' + this.robotId,
		class: 'buttons'
	}).appendTo('#robotControls');

	$('<div><ul id="controller-state-' + this.robotId +
		'" class="controller-state"></ul></div>').appendTo('#robotControls');

	var self = this,
		keyboardEnabled = $.cookie('keyboard-enabled'),
		joystickEnabled = $.cookie('joystick-enabled');

	$('<button/>', {
		text: 'STOP',
		id: 'stop-btn-' + this.robotId,
		class: 'live-only',
		disabled: 'disabled',
		click: (function() {
			self.socket.send('<stop>');
		})
	}).appendTo('#buttons-' + this.robotId );

	$('<button/>', {
		text: self.config.socket.host,
		id: "host-btn-" + this.robotId
	}).appendTo('#buttons-' + this.robotId);

    $('#host-btn-' + this.robotId).click( function() {
        var newHost;

        newHost = window.prompt('Enter ' + self.robotId +  ' hostname or IP', self.config.socket.host);
        if (typeof(newHost) == 'string' && newHost.length > 0) {
            self.config.socket.host = newHost;
            self.socket.open(self.config.socket.host, self.config.socket.port);
            $.cookie('host-' + self.robotId, self.config.socket.host);
            $(this).html(self.config.socket.host);
        }
    });

    $('<button/>', {
        text: 'State info',
        id: 'state-info-btn-' + this.robotId,
		class: 'live-only',
        disabled: 'disabled'
    }).appendTo('#buttons-' + this.robotId);
    $('#state-info-btn-' + this.robotId).click( function() {
        self.showCurrentStateInfo();
        return false;
    }).bind('clickoutside', function() {
        self.hideStateInfo();
    });

	$('<button/>', {
		text: 'Show Video',
		id: 'fetch-frame-btn-' + this.robotId,
		class: "live-only",
		disabled: 'disabled',
		click: (function() {
			dash.ui.showModal('camera-view');
			self.socket.send('<get-frame>');
			self.socket.send('<list-screenshots>');
		})
	}).appendTo('#buttons-' + this.robotId);

	$('<button/>', {
		text: 'Calibrate lens',
		id: 'toggle-camera-translator-btn-' + this.robotId,
		class: "live-only",
		disabled: 'disabled',
		click: (function() {
			dash.ui.showModal('camera-translator');
		})
	}).appendTo('#buttons-' + this.robotId);

    var table = $('<table style="width: 100%;"/>').addClass('ctrl manual-ctrl test-ctrl');
    var row1 = $('<tr><th>Keyboard</th><td><input type="checkbox" name="keyboard-controller-enabled-'+this.robotId+'" value="1"/></td></tr>');
	var row2 = $('<tr><th>Joystick</th><td><input type="checkbox" name="joystick-controller-enabled" value="1"/></td></tr>');
	//$('<input />', { type: 'checkbox', id: 'cb1', name: 'keyboard-controller-enabled', value: 1 }).appendTo('#buttons-' + this.robotId);
	table.append(row1);
	table.append(row2);
    $('#buttons-' + this.robotId).append(table);

	var sel = $('<select/>', {
		name: "controller-choice",
		id: "controller-choice",
		class: "live-only seperated",
		disabled: "disabled"
	}).appendTo('#buttons-' + this.robotId);

	$("<option />", {value: "", text: "-- Select Controller --"}).appendTo(sel);
	$("<option />", {value: "test", text: "Test Controller"}).appendTo(sel);
	$("<option />", {value: "offensive-ai", text: "Offensive AI"}).appendTo(sel);

	this.keyboardController.enabled = parseInt(keyboardEnabled) == 1;
	this.joystickController.enabled = parseInt(joystickEnabled) == 1;
	//row1.prop('checked', this.keyboardController.enabled);
	//$('INPUT[name="keyboard-controller-enabled"]').prop('checked', this.keyboardController.enabled);
	$('INPUT[name="joystick-controller-enabled"]').prop('checked',  this.joystickController.enabled);

	window.setTimeout(function() {
		$('INPUT[name="keyboard-controller-enabled-'+self.robotId+'"]').iphoneStyle({
			onChange: function(elem, enabled) {
				self.keyboardController.enabled = enabled;
				$.cookie('keyboard-enabled', enabled ? 1 : 0);
			}
		});

		$('INPUT[name="joystick-controller-enabled"]').iphoneStyle({
			onChange: function(elem, enabled) {
				self.joystickController.enabled = enabled;
				$.cookie('joystick-enabled', enabled ? 1 : 0);

				if (enabled) {
					$('INPUT[name="keyboard-controller-enabled"]')
						.removeAttr('checked')
						.iphoneStyle('refresh')
						.attr('disabled', 'disabled')
						.iphoneStyle('refresh');
				} else {
					$('INPUT[name="keyboard-controller-enabled"]')
						.removeAttr('disabled')
						.iphoneStyle('refresh');
				}
			}
		});
	}, 500);

	this.joystickController.onSelfEnable = function() {
		$('INPUT[name="keyboard-controller-enabled"]')
			.removeAttr('checked')
			.iphoneStyle('refresh')
			.attr('disabled', 'disabled')
			.iphoneStyle('refresh');
	};

	$('#controller-choice').change(function() {
		var controller = $(this).val();
		
		self.setController(controller);
	});

	$('<button/>', {
		text: "Reset position",
		id: "reset-position-btn-" + this.robotId,
		class: "live-only ctrl manual-ctrl",
		disabled: "disabled",
		click: (function() {
			self.robot.resetPosition();
		})
	}).appendTo('#buttons-' + this.robotId);

	$('<button id="toggle-dribbler-btn-' + this.robotId +
		'" class="live-only ctrl manual-ctrl" disabled="disabled">Toggle Dribbler</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="kick-btn-"' + this.robotId +
		'" class="live-only ctrl manual-ctrl" disabled="disabled">Kick</button>'
	).appendTo('#buttons-' + this.robotId);

			<!-- test controller -->
	$('<button id="manual-control-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-manual-control" disabled="disabled">Manual Control</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="watch-ball-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-watch-ball" disabled="disabled">Watch Ball</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="watch-goal-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-watch-goal" disabled="disabled">Watch Goal</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="watch-goal-behind-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-watch-goal-behind" disabled="disabled">Watch Goal Behind</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="spin-around-dribbler-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-spin-around-dribbler" disabled="disabled">Spin around dribbler</button>'
	).appendTo('#buttons-' + this.robotId);
	/*$('<button id="fetch-ball-infront-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn" data-cmd="run-fetch-ball-infront" disabled="disabled">Fetch ball in-front</button>'
	).appendTo('#buttons-' + this.robotId);*/
	$('<button id="find-ball-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-find-ball" disabled="disabled">Find ball</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="fetch-ball-front-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-fetch-ball-front" disabled="disabled">Fetch ball front</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="fetch-ball-direct-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-fetch-ball-direct" disabled="disabled">Fetch ball direct</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="fetch-ball-behind-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-fetch-ball-behind" disabled="disabled">Fetch ball behind</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="fetch-ball-near-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-fetch-ball-near" disabled="disabled">Fetch ball near</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="drive-circle-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-drive-circle" disabled="disabled">Drive Circle</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="accelerate-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-accelerate" disabled="disabled">Approach accelerated</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="escape-corner-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-escape-corner" disabled="disabled">Escape corner</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="drive-home-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-drive-home" disabled="disabled">Drive home</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="return-field-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="run-return-field" disabled="disabled">Return field</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="reset-position-btn-' + this.robotId + '" class="live-only ctrl test-ctrl ' +
		'send-cmd-btn' + this.robotId + '" data-cmd="reset-position" disabled="disabled">Reset position</button>'
	).appendTo('#buttons-' + this.robotId);

	$('<button id="drive-to-btn-' + this.robotId + '" class="live-only ctrl test-ctrl" ' +
		'disabled="disabled">Drive to</button>'
	).appendTo('#buttons-' + this.robotId);
	$('<button id="turn-by-btn-' + this.robotId + '" class="live-only ctrl test-ctrl" ' +
		'disabled="disabled">Turn by</button>'
	).appendTo('#buttons-' + this.robotId);


    $('<button/>', {
        text: "Rebuild",
        id: "rebuild-btn" + this.robotId,
        click: (function() {
            var btn = $(this);

            btn.html('Building..').attr('disabled', 'disabled');

            self.rebuild(function() {
                btn.removeAttr('disabled').html('Rebuild');
            });
        })
    }).appendTo('#buttons-' + this.robotId);

    $('<button/>', {
        text: "Kill",
        id: "kill-btn" + this.robotId,
	    click: (function() {
		var btn = $(this);
		
		btn.html('Killing it..').attr('disabled', 'disabled');
		
		self.kill(function() {
			btn.removeAttr('disabled').html('Kill');
		});
	})
    }).appendTo('#buttons-' + this.robotId);

	$('#shutdown-btn-' + this.robotId).click(function() {
		var btn = $(this);
		
		btn.html('Shutting down..').attr('disabled', 'disabled');
		
		self.shutdown(function() {
			btn.removeAttr('disabled').html('Shutdown');
		});
	});
	
	$('#calibrate-camera-btn-' + this.robotId).click(function() {
        self.socket.send('<get-camera-calibration>');
	});
	
	$('#calibrate-blobber-btn-' + this.robotId).click(function() {
		var selectedClass = $('#blobber-class').val();

        self.socket.send('<get-blobber-calibration:' + selectedClass + '>');
	});
	




	$('#show-blobber-btn-' + this.robotId).click(function() {
		dash.ui.showModal('blobber-view');
	});
	
	$('#blobber-class').change(function() {
		var selectedClass = $('#blobber-class').val();

        self.socket.send('<get-blobber-calibration:' + selectedClass + '>');
	});
	

	
	$('#kick-btn-' + this.robotId).click(function() {
		self.robot.kick();
	});
	
	$('#toggle-dribbler-btn-' + this.robotId).click(function() {
		self.robot.toggleDribbler();
	});
	


	$('#drive-to-btn-' + this.robotId).click(function() {
		dash.renderer.showDriveTo(self.robotIndex);

	});

	$('#turn-by-btn-' + this.robotId).click(function() {
		var angle = window.prompt('Enter angle', 90.0);

        self.socket.send('<turn-by:' + angle +'>');
	});

	$(window).keydown(function(e) {
		if (e.keyCode == 27) {
            self.socket.send('<stop>');
		}
	});

	$('.send-cmd-btn'+ this.robotId).click(function() {
        self.socket.send('<' + $(this).data('cmd') + '>');
	});

	$('.send-parameter-field').keyup(function() {
		$('.send-parameter-field').each(function() {
			var index = $(this).data('index'),
				value = parseFloat($(this).val());

			if (!isNaN(value)) {
				window.localStorage['parameter-' + index] = value;
			}
		});

		var value = parseFloat($(this).val());

		if (!isNaN(value)) {
            self.socket.send('<parameter:' + $(this).data('index') + ':' + value + '>');
		}
	});
	
	/*$('#test-turn-btn').click(function() {
		self.robot.turnBy(Math.PI / 2.0, 2);
	});
	
	$('#test-drive-to-btn').click(function() {
		self.robot.driveTo(0.125, 0.125, 0, 0.5);
	});
	
	$('#test-drive-back-btn').click(function() {
		self.robot.driveTo(2.0, 0.125, Math.PI, 0.5);
		self.robot.driveTo(0.125, 0.125, 0, 0.5);
	});
	
	$('#test-rectangle-btn').click(function() {
		self.robot.testRectangle();
	});
	
	$('#test-drive-facing-btn').click(function() {
		self.robot.driveFacing(3, 2, 4.5, 1.5, 1);
	});
	
	$('#test-watch-ball-btn').click(function() {
		this.socket.send('<test-watch-ball>');
	});
	
	$('#test-chase-ball-btn').click(function() {
		this.socket.send('<test-chase-ball>');
	});
	
	$('#test-find-goal-btn').click(function() {
		this.socket.send('<test-find-goal>');
	});*/
	
	$('#graphs-toggle-btn').click(function() {
		if ($('#wheel-graphs').hasClass('full')) {
			$('#wheel-graphs').removeClass('full');
			$('#graphs-toggle-btn').show();
		} else {
			$('#wheel-graphs').addClass('full');
			$('#graphs-toggle-btn').hide();
			
			window.setTimeout(function() {
				$('#wheel-graphs').one('clickoutside', function() {
					$(this).removeClass('full');
					$('#graphs-toggle-btn').show();
				});
			}, 500);
		}
		
		return false;
	});
	
	// camera calibration
	$('#camera-exposure').slider('change', function(value) {
        self.socket.send('<camera-set-exposure:' + value + '>');
	});
	
	$('#camera-gain').slider('change', function(value) {
        self.socket.send('<camera-set-gain:' + value + '>');
	});
	
	$('#camera-red, #camera-green, #camera-blue').slider('change', function() {
		var red = $('#camera-red').val(),
			green = $('#camera-green').val(),
			blue = $('#camera-blue').val();

        self.socket.send('<camera-set-white-balance:' + red + ':' + green + ':' + blue + '>');
	});
	
	$('#camera-luminosity-gamma').slider('change', function(value) {
        self.socket.send('<camera-set-luminosity-gamma:' + (value) + '>');
	});
	
	$('#camera-chromaticity-gamma').slider('change', function(value) {
        self.socket.send('<camera-set-chromaticity-gamma:' + (value) + '>');
	});
	
	// blobber
	$('#blobber-y, #blobber-u, #blobber-v, #blobber-merge-threshold').slider('change', function() {
		var selectedClass = $('#blobber-class').val(),
			y = $('#blobber-y').val().replace(' ', ','),
			u = $('#blobber-u').val().replace(' ', ','),
			v = $('#blobber-v').val().replace(' ', ','),
			mergeThreshold = $('#blobber-merge-threshold').val();

        self.socket.send('<set-blobber-calibration:' + selectedClass + ':' + y + ':' + u + ':' + v + ':' + mergeThreshold + '>');
	});
	
	$('#frame-img, #frame-classification, #frame-canvas').bind('contextmenu', function(e) {
		e.preventDefault();
	});

	$('#camera-opacity').on('input', function() {
		$('#frame-classification').css('opacity', $(this).val() / 100);
	});
	
	$('#frame-img, #frame-classification, #frame-canvas').mousedown(function(e) {
		var sizeMult = 1,
			//sizeMult = 2,
			x = e.offsetX % (1280 / sizeMult) * sizeMult, // times two since the image is half size
			y = e.offsetY * sizeMult,
			mode = 2,
			color = $('#threshold-class').val(),
			brush = $('#threshold-brush').val(),
			stdev = $('#threshold-stdev').val();

		if (color === '') {
			return;
		}
		
		switch (e.which) {
			case 1:
				mode = 2;
			break;
			
			case 2:
				mode = 1;
			break;
			
			case 3:
				mode = 3;
			break;
		}

        self.socket.send('<blobber-threshold:' + color + ':' + x + ':' + y + ':' + mode + ':' + brush + ':' + stdev + '>');
        self.socket.send('<get-frame>');
		
		e.preventDefault();
	});
	
	$('#blobber-clear-current-btn').click(function() {
        self.socket.send('<blobber-clear:' + $('#threshold-class').val() + '>');
	});
	
	$('#blobber-clear-all-btn').click(function() {
        self.socket.send('<blobber-clear>');
	});

	$('#screenshot-btn').click(function() {
        self.socket.send('<screenshot:' + $('#screenshot-filename').val().replace('-', '_') + '>');
	});
	
	$('#ai-toggle-go-btn').click(function() {
        self.socket.send('<toggle-go>');
	});

	$('#ai-toggle-side-btn').click(function() {
        self.socket.send('<toggle-side>');
	});

	$('#status').click(function() {
		self.toggleTargetSide();
	});
	
	$('#camera-choice').change(function() {
        self.socket.send('<camera-choice:' + $(this).val()+ '>');
	});

	$('#stream-choice').change(function() {
        self.socket.send('<stream-choice:' + $(this).val()+ '>');

		self.applyScreenshotState();
	});

	$('#camera-k, #camera-zoom').keyup(function() {
		var k = parseInt($('#camera-k').val()) / 1000000000,
			zoom = parseInt($('#camera-zoom').val()) / 1000;

        self.socket.send('<camera-adjust:' + k + ':' + zoom + '>');
	});
};

Dash.UI.prototype.initBlobberView = function() {
	this.blobberView = new Dash.BlobberView();
	this.blobberView.init();
};

Dash.UI.prototype.initFrameCanvas = function() {
	this.frameCanvas = new Dash.FrameCanvas();
	this.frameCanvas.init();
};

Dash.UI.prototype.applyScreenshotState = function() {
	if ($('#stream-choice').val() === '') {
		$('#screenshot-filename').attr('disabled', false);
		$('#screenshot-btn').attr('disabled', false);
	} else {
		$('#screenshot-filename').attr('disabled', 'disabled');
		$('#screenshot-btn').attr('disabled', 'disabled');
	}
};

Dash.UI.prototype.toggleTargetSide = function() {
	if (this.states.length == 0) {
		return;
	}
	
	var lastState = this.states[this.states.length - 1];

	this.socket.send('<toggle-side>');
};

Dash.UI.prototype.driveTo = function(x, y, orientation) {
	this.socket.send('<drive-to:' + x + ':' + y+ ':' + orientation + '>');
};

Dash.UI.prototype.setController = function(name) {
	/*$('.ctrl').each(function() {
		if ($(this).hasClass(name + '-ctrl')) {
			$(this).show();
		} else {
			$(this).hide();
		}
	});*/

	$('.ctrl').each(function() {
		$(this).hide();
	});
	
	this.robot.setController(name);
};

Dash.UI.prototype.onKeyDown = function(e) {
	//dash.dbg.log('! Key down: ' + e.keyCode);
	
	if (this.keyboardController.enabled) {
		this.keyboardController.onKeyDown(e.keyCode);
	}
};

Dash.UI.prototype.onKeyUp = function(e) {
	//dash.dbg.log('! Key up: ' + e.keyCode);
	
	if (this.keyboardController.enabled) {
		this.keyboardController.onKeyUp(e.keyCode);
	}
};

Dash.UI.prototype.isKeyDown = function(key) {
	return typeof(this.keystates[key]) != 'undefined' && this.keystates[key] == true;
};

Dash.UI.prototype.handleMessage = function(message,robotId) {
	if (typeof(message.id) != 'string') {
		dash.dbg.log('- Unknown message', message);
		return;
	}

	switch (message.id) {
		case 'controller':
			this.handleControllerMessage(message.payload);
		break;
		
		case 'state':
			this.handleStateMessage(message.payload);
		break;
		
		case 'log':
			this.handleLogMessage(message.payload);
		break;
		
		case 'camera-calibration':
			this.handleCameraCalibrationMessage(message.payload);
		break;
		
		case 'blobber-calibration':
			this.handleBlobberCalibrationMessage(message.payload);
		break;
		
		case 'frame':
			this.handleFrameMessage(message.payload);
		break;

		case 'screenshots':
			this.handleScreenshotsMessage(message.payload);
		break;
		
		default:
			dash.dbg.log('- Unsupported message received: ' + message.id);
		break;
	}
	
	this.rxCounter.step();
};

Dash.UI.prototype.handleControllerMessage = function(controller) {
	dash.dbg.log('! Received active controller: ' + controller);
	
	$('#controller-choice').val(controller);
		
	this.robot.controller = controller;

	$('.ctrl').each(function() {
		if ($(this).hasClass(controller + '-ctrl')) {
			$(this).show();
		} else {
			$(this).hide();
		}
	});

	this.socket.send('<get-state>'); //Get first state

};

Dash.UI.prototype.handleStateMessage = function(state) {
    if(dash.commonLogic.connectedRobots.indexOf(state.robot_id) == -1) {
        dash.commonLogic.connectedRobots.push(state.robot_id);
        dash.commonLogic.connectedUIs.push(this);
    }

    this.addState(state);

    this.socket.send('<get-state>'); // request for new state
};

Dash.UI.prototype.handleLogMessage = function(messages) {
	var lines = messages.split(/\n/g),
		i;
	
	for (i = 0; i < lines.length; i++) {
		if (lines[i].length > 0) {
			dash.dbg.external(lines[i]);
		}
	}
};

Dash.UI.prototype.handleCameraCalibrationMessage = function(calibration) {
	dash.dbg.console('camera calibration', calibration);
	
	$('#camera-exposure').slider('val', parseInt(calibration.exposure));
	$('#camera-gain').slider('val', parseInt(calibration.gain));
	
	this.showModal('camera-calibration');
};

Dash.UI.prototype.handleBlobberCalibrationMessage = function(calibration) {
	dash.dbg.console('blobber calibration', calibration);
	
	$('#blobber-y').slider('val', calibration.yLow, calibration.yHigh);
	$('#blobber-u').slider('val', calibration.uLow, calibration.uHigh);
	$('#blobber-v').slider('val', calibration.vLow, calibration.vHigh);
	$('#blobber-merge-threshold').slider('val', calibration.mergeThreshold);
	
	this.showModal('blobber-calibration');
};

Dash.UI.prototype.handleFrameMessage = function(frame) {
	if (!$('#camera-view').is(':visible')) {
		return;
	}

	$('#frame-img').attr('src', '');
	$('#frame-img').attr('width', '0');
	$('#frame-img').attr('height', '0');
	$('#frame-img').attr('width', '640');
	$('#frame-img').attr('height', '512');

	$('#frame-img').attr('src', 'data:image/jpeg;base64,' + frame.rgb);
	$('#frame-classification').attr('src', 'data:image/jpeg;base64,' + frame.classification);
	$('#stream-choice OPTION.selected').attr('selected', false);
	$('#stream-choice OPTION[value=' + frame.activeStream + ']').attr('selected', 'selected');

	if (!$('#camera-k').is(':focus') && !$('#camera-zoom').is(':focus')) {
		$('#camera-k').val(parseInt(parseFloat(frame.cameraK) * 1000000000)).attr('disabled', false);
		$('#camera-zoom').val(parseFloat(parseInt(parseFloat(frame.cameraZoom) * 1000))).attr('disabled', false);
	}

	this.applyScreenshotState();

	this.socket.send('<get-frame>');
};


Dash.UI.prototype.handleScreenshotsMessage = function(screenshots) {
	var select = $('#stream-choice'),
		i;

	select.find('OPTION:gt(0)').remove();

	for (i = 0; i < screenshots.length; i++) {
		if (screenshots[i] === '') {
			continue;
		}

		select.append('<option value="' + screenshots[i] + '">' + screenshots[i] + '</option>');
	}
};

Dash.UI.prototype.showModal = function(id) {
	$('.modal[id!="' + id + '"]').fadeOut(100);
	
	$('#' + id).fadeIn(150, function() {
		$(this).one('clickoutside', function() {
			$(this).fadeOut();
		});
	});
};

Dash.UI.prototype.addState = function(state) {
	/*if (this.states.length >= 100000) {
		this.states = [];
	}*/

	var full = false,
		live = this.states.length <= 1 ||
            dash.commonLogic.currentStateIndex == dash.commonLogic.states.length - 1;

	if (!live) {
		return;
	}

	state.index = this.states.length;
	
	if (this.states.length > 0) {
		state.previous = this.states[this.states.length - 1];
	} else {
		state.previous = null;
	}

	if (this.states.length >= 10000) {
		this.states.shift();
		full = true;
	}



    //if(dash.commonLogic.showStateActive == false) {
        var dTime = new Date();
        state["clockMillis"] = dTime.getTime();
        this.states.push(state);
        //dash.commonLogic.allStates[state.robot_id].push(state);
        dash.commonLogic.states.push(state);
    //}
	// disable slider
	//this.stateSlider.slider('max', this.states.length);
	//this.stateCountWrap.html(this.states.length);
	dash.commonLogic.stateSlider.slider('max', dash.commonLogic.states.length);
	dash.commonLogic.stateCountWrap.html(dash.commonLogic.states.length);

	if (state.playing) {
		$('#ai-toggle-go-btn').html('Stop');
		$('#ai-toggle-side-btn').attr('disabled', 'disabled');
	} else {
		$('#ai-toggle-go-btn').html('Start');
		$('#ai-toggle-side-btn').attr('disabled', false);
	}
	
	if (live) {
		//this.showState(this.states.length - 1);
		dash.commonLogic.showState(dash.commonLogic.states.length - 1);
	}

	if (!this.extractedCameraTranslator) {
		this._loadCameraTranslator(
			state.frontCameraTranslator,
			state.rearCameraTranslator
		);

		this.extractedCameraTranslator = true;
	}
};

Dash.UI.prototype._loadCameraTranslator = function(front, rear) {
	console.log('translator', front, rear);

	var constants = front,
		$table = $('#camera-translator-table'),
		rangeMultiplier = 4,
		constantName,
		constantValue,
		minValue,
		maxValue,
		step,
		temp;

	$table.empty();

	for (constantName in constants) {
		constantValue = constants[constantName];
		maxValue = constantValue * rangeMultiplier;
		minValue = constantValue - constantValue * rangeMultiplier;
		step = Math.abs(constantValue) / 100;

		if (maxValue < minValue) {
			temp = maxValue;
			maxValue = minValue;
			minValue = temp;
		}

		$table.append(
			'<tr>' +
			'	<td>' + constantName + '</td>' +
			'	<td><input class="range-slider camera-translator-constant" data-name="' + constantName + '" type="range" value="' + constantValue * 1000 + '" min="' + minValue * 1000 + '" max="' + maxValue * 1000 + '" step="' + step * 1000 + '"></td>' +
			'	<td class="camera-translator-constant-value-' + constantName + '">' + constantValue.toPrecision(4) + '</td>' +
			'</tr>'
		);
	}

	var $sliders = $('.camera-translator-constant');

	$sliders.on('input', function() {
		var constantName = $(this).data('name'),
			value = (parseFloat($(this).val()) / 1000).toPrecision(5),
			$valueWrap = $('.camera-translator-constant-value-' + constantName + ''),
			values = [];

		$valueWrap.html(value);

		$sliders.each(function() {
			values.push(parseFloat($(this).val()) / 1000);
		});

		dash.ui.robot.setCameraTranslatorConstants.apply(dash.ui.robot, values);

		console.log(values);
	});
};

Dash.UI.prototype.showState = function(index) {
	if (typeof(this.states[index]) == 'undefined') {
		return;
	}
	
	var state = this.states[index];

	// disable slider
	//this.currentStateIndexWrap.val(index + 1);
	//this.stateSlider.slider('val',index + 1);
	
	//this.currentStateIndex = index;
	
	// @TODO show alive wheels..
	
	//dash.renderer.renderState(state);
	
	//this.showTasksQueue(state);
	//this.showStateStats(state); // @TEMP
};

Dash.UI.prototype.showCurrentStateInfo = function() {
	if (this.states.length == 0) {
		return;
	}
	
	var currentState = this.states[this.currentStateIndex];
	
	this.showStateInfo(currentState);
};

Dash.UI.prototype.showStateInfo = function(state) {
	var showState = $.extend({}, state);
	delete showState.previous;
	
	$('#state-info').html(Dash.Util.highlightJSON(showState)).fadeIn();
};

Dash.UI.prototype.showCalibrateCamera = function(state) {
	$('#camera-calibration').fadeIn();
};

Dash.UI.prototype.hideStateInfo = function() {
	$('#state-info').html('').hide();
};

Dash.UI.prototype.flashClass = function(el, className, duration) {
	if (typeof(el) == 'string') {
		el = $(el);
	}
	
	var timeout = el.data('flash-timeout');
	
	if (timeout != null) {
		window.clearTimeout(timeout);
	}
	
	if (!el.hasClass(className)) {
		el.addClass(className);
	}
	
	el.data('flash-timeout', window.setTimeout(function() {
		el.removeClass(className);
	}, duration));
};

Dash.UI.prototype.showTasksQueue = function(state) {
	var wrap = $('#tasks'),
		i;
	
	wrap.empty();
	
	for (i = 0; i < state.robot.tasks.length; i++) {
		wrap.append('<li><div class="percentage" style="width: ' + Math.ceil(state.robot.tasks[i].percentage) + '%;"></div><div class="status">' + state.robot.tasks[i].status + '<div></li>');
	}
};

Dash.UI.prototype.showStateStats = function(state) {
	/*for (var robotIndex = 0; robotIndex < dash.commonLogic.maxNrOfRobots; robotIndex++) {
		if(dash.commonLogic.connectedRobots.indexOf(state.robot_id) == -1) {
			var wrap = $('#controller-state-samott' + robotIndex);
			wrap.html('');
		}
	}
*/
	for (var i = 0; i < state.length; ++i) {
		$('#time').html(Dash.Util.round(state[i].totalTime, 1) + 's / ' + Dash.Util.round(state[i].dt * 1000, 1) + 'ms / ' + state[i].fps + 'FPS / ' + Math.round(this.rxCounter.getLastFPS()) + 'PPS');
		//$('#load > SPAN').css('width', Math.ceil(state[i].load) + '%');

		if (state[i].gotBall) {
			$('#ball-indicator').addClass('got-ball');
		} else {
			$('#ball-indicator').removeClass('got-ball');
		}

		if (state[i].robot.dribbler.isRaised) {
			$('#dribbler-indicator').removeClass().addClass('dribbler-raised');
		} else if (state[i].robot.dribbler.isLowered) {
			$('#dribbler-indicator').removeClass().addClass('dribbler-lowered');
		} else {
			$('#dribbler-indicator').removeClass().addClass('dribbler-moving');
		}

		//$('#fps-indicator').html(state[i].fps + ' FPS');

		if ($('#blobber-view').is(':visible')) {
			this.blobberView.render(state[i]);
		}

		$('#status').removeClass('yellow blue go stop');

		if (state[i].targetSide == 1) {
			$('#status').addClass('yellow');
		} else if (state[i].targetSide == 0) {
			$('#status').addClass('blue');
		}

		if (state[i].playing == 1) {
			$('#status').addClass('go');
		} else if (state[i].playing == 0) {
			$('#status').addClass('stop');
		}

		if (state[i].isError) {
			$('#contents').removeClass('no-error');
		} else {
			$('#contents').addClass('no-error');
		}
		var contState = state[i].controllerState;
		//contState.push();
		contState["dateTimeMillis"] = state[i].dateTimeMillis;
		contState["clockMillis"] = state[i].clockMillis;
        contState["robot_id"] = state[i].robot_id;
		this.showControllerState(contState);

		if (
			state[i].controllerState !== null
			&& state[i].controllerState.particleLocalizer !== null
			&& typeof(state[i].controllerState.particleLocalizer) !== 'undefined'
			&& typeof(state[i].controllerState.particleLocalizer.particles) !== 'undefined'
		) {
			delete state[i].controllerState.particleLocalizer.particles;
		}
	}

    //this.controllerStateLength = document.getelemen
};

Dash.UI.prototype.showControllerState = function(state) {

	var wrap = $('#controller-state-samott' + state.robot_id),
		entries = [],
		key,
		value,
		sub,
		parentId,
		i = 0;

	wrap.html('');

	if (state != null && typeof(state) == 'object') {
		for (key in state) {
			entries.push({
				key: key,
				value: state[key]
			});
		}
	}

	entries = entries.sort(function(a, b) {
		return a.key < b.key ? -1 : 1;
	});

	for (i = 0; i < entries.length; i++) {
		key = entries[i].key;
		value =  entries[i].value;

		if (typeof(value) === 'object') {
			parentId = 'controller-parent-' + i + "samott" + state.robot_id;

			wrap.append('<li id="' + parentId + '"><strong>' + key + '</strong><ul></ul></li>');

			for (sub in value) {
				if (sub === 'particles') {
					continue;
				}

				$('#' + parentId + ' UL').append('<li><strong>' + sub + '</strong>: ' + state[key][sub] + '</li>')
			}
		} else {
			wrap.append('<li><strong>' + key + '</strong>: ' + value + '</li>');
		}
	}

	$('#obstruction-indicator-left').removeClass('active').html(state.obstruction.invalidCountLeft);
	$('#obstruction-indicator-right').removeClass('active').html(state.obstruction.invalidCountRight);

	if (state.obstruction.left) {
		$('#obstruction-indicator-left').addClass('active');
	}

	if (state.obstruction.right) {
		$('#obstruction-indicator-right').addClass('active');
	}

	if (state.isKickingOnceGotBall) {
		$('#coilgun-indicator').addClass('active');
	} else {
		$('#coilgun-indicator').removeClass('active');
	}
};

Dash.UI.prototype.rebuild = function(callback) {
	this.request('rebuild', callback);
};

Dash.UI.prototype.kill = function(callback) {
	this.request('kill', callback);
};

Dash.UI.prototype.shutdown = function(callback) {
	this.request('shutdown', callback);
};

Dash.UI.prototype.request = function(action, callback) {
	$.ajax({
		url: 'http://' + this.socket.host + '/dash/soccerbot.php?action=' + action,
		type: 'GET',
		dataType: 'html',
		timeout: 120000
	}).success(function() {
		if (typeof(callback) == 'function') {
			callback(true);
		}
	}).fail(function() {
		if (typeof(callback) == 'function') {
			callback(false);
		}
		
		dash.dbg.log('- executing ' + action + ' failed');
	});
};

