$(document).ready(function() {
	window.dash = {
		config: Dash.Config,
		dbg: new Dash.Debug(),
		renderer: new Dash.Renderer(),
		socket: new Dash.Socket(Dash.Config.socket.socketId),
		socket2: new Dash.Socket(Dash.Config.socket2.socketId),
		ui: new Dash.UI(),
		ui2: new Dash.UI()
	};

	dash.dbg.init();
	dash.renderer.init();
	dash.ui.init(dash.socket, dash.config.robot.robotId);
});