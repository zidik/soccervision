$(document).ready(function() {
	window.dash = {
		config: Dash.Config,
		dbg: new Dash.Debug(),
		renderer: new Dash.Renderer(),
		socket: new Dash.Socket(),
		ui: new Dash.UI()
	};
	
	dash.dbg.init();
	dash.renderer.init();
	dash.ui.init();
});