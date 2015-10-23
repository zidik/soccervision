/* global module */
module.exports = function (grunt) {
	'use strict';

	// require all the dependencies
	require('matchdep').filterDev('grunt-*').forEach(grunt.loadNpmTasks);

	// set grunt config
	grunt.initConfig({
		pkg: grunt.file.readJSON('package.json'),

		// creates a local server for viewing the examples
		// https://github.com/gruntjs/grunt-contrib-connect
		connect: {
			dash: {
				options: {
					hostname: '0.0.0.0',
					port: 8084,
					base: './',
					keepalive: true
				}
			}
		}
	});

	grunt.registerTask('default', ['connect:dash']);
};