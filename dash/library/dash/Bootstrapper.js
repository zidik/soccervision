$(document).ready(function() {
	window.dash = {
		config: Dash.Config,
		dbg: new Dash.Debug(),
		renderer: new Dash.Renderer(),
		socket: new Dash.Socket(Dash.Config.socket),
		socket2: new Dash.Socket(Dash.Config.socket2),
		ui: new Dash.UI(),
		ui2: new Dash.UI()
	};

	dash.dbg.init();
	dash.renderer.init();
	dash.ui.init(dash.socket, dash.config.robot.robotId, Dash.Config.socket.Events,dash.config.controls);
	dash.ui2.init(dash.socket2, dash.config.robot2.robotId, Dash.Config.socket2.Events,dash.config.controls2);
});