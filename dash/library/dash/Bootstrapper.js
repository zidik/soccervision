$(document).ready(function() {
	window.dash = {
		config: new Dash.Config(),
		dbg: new Dash.Debug(),
		renderer: new Dash.Renderer(),
		socket: new Dash.Socket(),
		ui: new Dash.UI(),
		commonLogic: new Dash.CommonLogic()
	};

	dash.config.init(0);
	dash.dbg.init();
	dash.renderer.init();
	dash.socket.init(0);
	dash.ui.init(dash.socket, dash.config);
	dash.commonLogic.init();

	window.dash2 = {
		config: new Dash.Config(),
		dbg: new Dash.Debug(),
		renderer: new Dash.Renderer(),
		socket: new Dash.Socket(),
		ui: new Dash.UI()
	};
	dash2.config.init(1);
	//dash2.dbg.init();
	//dash2.renderer.init();
	dash2.socket.init(1);
	dash2.ui.init(dash2.socket, dash2.config);

});