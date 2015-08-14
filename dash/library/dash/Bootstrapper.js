$(document).ready(function() {
	window.dash = {
		config: new Dash.Config(),
		dbg: new Dash.Debug(),
		renderer: new Dash.Renderer(),
		socket: new Dash.Socket(),
		//socket2: new Dash.Socket(Dash.Config.socket2),
		ui: new Dash.UI()
		//ui2: new Dash.UI()
	};
	dash.config.init(1);
	dash.dbg.init();
	dash.renderer.init();
	dash.socket.init(1);
	dash.ui.init(dash.socket, dash.config.robot.robotId);
	//dash.ui2.init(dash.socket2, dash.config.robot2.robotId, Dash.Config.socket2.Events,dash.config.controls2);
});