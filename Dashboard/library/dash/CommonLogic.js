/**
 * Created by Tonnius on 29.09.2015.
 */
Dash.CommonLogic = function() {
    this.allStateBundles = [];
    this.states = [];
    this.stateSlider = null;
    this.currentStateIndexWrap = null;
    this.currentStateIndex = 0;
    this.stateCountWrap = null;
    this.showStateActive = false;
    this.connectedRobots = [];
	this.allConnectedRobotsThisSession = [];
    this.connectedUIs = [];
	
    this.maxNrOfRobots = 2;
    this.disconnectedRobots = [];
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
			if(this.currentStateIndex == this.states.length - 1)
				self.showState(value - 1);
			else
				self.showStateBundle(value - 1);
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

        if(this.currentStateIndex == this.states.length - 1)
				self.showState(value - 1);
			else
				self.showStateBundle(value - 1);
    });

    $('.slider').slider({
        showRange: true,
        showValue: true,
        width: 300,
        minChangeInterval: 500
    });
};

Dash.CommonLogic.prototype.showState = function(index) {
    if (typeof(this.states[index]) == 'undefined') {
        return;
    }

    this.showStateActive = true;
    var oneStateBundle = [];
    var robotId = this.states[index].robot_id;
    var state = null;
    var maxRobots = Math.max(...this.connectedRobots);
    for (var robotIndex = 0; robotIndex < this.allConnectedRobotsThisSession.length; robotIndex++) {
        if(robotIndex == robotId /*&& this.connectedRobots.indexOf(robotIndex) > -1*/) {
            oneStateBundle.push(this.states[index]);
            state = this.states[index];
            state.controllerState["decidingState"] = 1;
        }
        else {
            //if (this.connectedRobots.indexOf(robotIndex) > -1) {
                var timeMillisPrevious;
                var previousStateIndex = -1;
                var timeMillis;
                //var allStatesLength = this.allStates[robotIndex].length - 1;

                for (var stateIndex = index - 1; stateIndex >= 0; stateIndex--) {
                    if (this.states[stateIndex].robot_id == robotIndex) {
                        timeMillisPrevious = this.states[stateIndex].clockMillis;
                        timeMillis = this.states[index].clockMillis;
                        if (timeMillisPrevious <= timeMillis) {
                            previousStateIndex = stateIndex;
                            break;
                        }
                    }

                }

                if (previousStateIndex != -1) {
                    oneStateBundle.push(this.states[previousStateIndex]);
                    state = this.states[previousStateIndex];
                    state.controllerState["decidingState"] = 0;
                }
                /*else {
                 oneStateBundle.push([]);
                 }*/

            //}
        }
    }
    if(oneStateBundle != null) {
        //this.connectedUIs[robotIndex].showTasksQueue(state);
        dash.ui.showStateStats(oneStateBundle); // @TEMP
		dash.renderer.renderState(oneStateBundle);
		this.allStateBundles.push(oneStateBundle);
    }
        //this.stateSlider.slider('max', this.allStates[0].length);
        //this.stateCountWrap.html(this.allStates[0].length);
        //state = this.states[index];

        // disable slider
        this.currentStateIndexWrap.val(index + 1);
        this.stateSlider.slider('val', index + 1);

        this.currentStateIndex = index;

    // @TODO show alive wheels..
    this.showStateActive = false;
    

    //this.showTasksQueue(state);
    //this.showStateStats(state); // @TEMP


};

Dash.CommonLogic.prototype.showStateBundle = function(index) {
	    if(oneStateBundle != null) {
        //this.connectedUIs[robotIndex].showTasksQueue(state);
        dash.ui.showStateStats(this.allStateBundles[index]); // @TEMP
		dash.renderer.renderState(this.allStateBundles[index]);
    }
        //this.stateSlider.slider('max', this.allStates[0].length);
        //this.stateCountWrap.html(this.allStates[0].length);
        //state = this.states[index];

        // disable slider
        this.currentStateIndexWrap.val(index + 1);
        this.stateSlider.slider('val', index + 1);

        this.currentStateIndex = index;
	
} 