/**
 * Created by Tonnius on 29.09.2015.
 */
Dash.CommonLogic = function() {
    this.allStates = [[],[]];
    this.stateSlider = null;
    this.currentStateIndexWrap = null;
    this.currentStateIndex = 0;
    this.stateCountWrap = null;

    this.connectedRobots = 0;
};

Dash.CommonLogic.prototype = new Dash.Bindable();

Dash.CommonLogic.prototype.init = function() {
    this.initSlider();
};

Dash.CommonLogic.prototype.initSlider = function() {
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

        if (newIndex > self.allStates.length - 1) {
            newIndex = self.allStates.length - 1;
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

Dash.CommonLogic.prototype.showState = function(index) {
    if (typeof(this.allStates[0][index]) == 'undefined') {
        return;
    }

    var state = this.allStates[0][index];

    // disable slider
    this.currentStateIndexWrap.val(index + 1);
    this.stateSlider.slider('val',index + 1);

    this.currentStateIndex = index;

    // @TODO show alive wheels..

    dash.renderer.renderState(state);

    //this.showTasksQueue(state);
    //this.showStateStats(state); // @TEMP
};