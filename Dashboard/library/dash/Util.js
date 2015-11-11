Dash.Util = {};

Dash.Util.getMicrotime = function() {
	return (new Date()).getTime() / 1000.0;
};

Dash.Util.getTime = function() {
	var date = new Date();

	return Dash.Util.formatTime(date);
};

Dash.Util.formatTime = function(date, includeSeconds) {
	date = date || new Date();
	includeSeconds = typeof(includeSeconds) != 'undefined'
		? includeSeconds
		: true;
	
	return (date.getHours() < 10 ? '0' : '') + date.getHours() +
		':' + (date.getMinutes() < 10 ? '0' : '') + date.getMinutes() +
		(includeSeconds ? ':' + (date.getSeconds() < 10 ? '0' : '') +
		date.getSeconds() : '');
};

Dash.Util.highlightJSON = function(json) {
    if (typeof json != 'string') {
         json = JSON.stringify(json, undefined, 2);
    }
	
    json = json.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
    
	return json.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, function (match) {
        var cls = 'number';
        if (/^"/.test(match)) {
            if (/:$/.test(match)) {
                cls = 'key';
            } else {
                cls = 'string';
            }
        } else if (/true|false/.test(match)) {
            cls = 'boolean';
        } else if (/null/.test(match)) {
            cls = 'null';
        }
        return '<span class="' + cls + '">' + match + '</span>';
    });
};

Dash.Util.round = function(number, decimals) {
	if (typeof(number) != 'number') {
		return number;
	}
	
	return number.toFixed(decimals);
};

Dash.Util.degToRad = function(degrees) {
	return degrees * Math.PI / 180.0;
};

Dash.Util.radToDeg = function(radians) {
	return radians * 180.0 / Math.PI;
};

Dash.Util.getDistanceBetween = function(a, b, squared) {
	var squaredValue = Math.pow(a.x - b.x, 2) + Math.pow(a.y - b.y, 2);

	if (squared) {
		return squaredValue;
	} else {
		return Math.sqrt(squaredValue);
	}
};

Dash.Util.map = function(value, inMin, inMax, outMin, outMax) {
	if (value < inMin) {
		return outMin;
	} else if (value > inMax) {
		return outMax;
	}

	return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
};

Dash.Util.getSpeed = function(currentSpeed, targetSpeed, dt, acceleration){
	var changeToTarget = targetSpeed - currentSpeed;
	var maxChange = acceleration * dt;

	var change;

	if (changeToTarget > 0){
		change = Math.min(maxChange, changeToTarget);
	}
	else{
		change = Math.max(-maxChange, changeToTarget);
	}

	return currentSpeed + change;
}