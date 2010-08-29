/* Copyright 2009 Palm, Inc.  All rights reserved. */

function AppAssistant(appController) {
  	console.info("AppAssistant")
  	this.appController = appController;
}

function StageAssistant(stageController) {
  	var queryParams = document.URL.toQueryParams();
  	this.stageController = stageController;
	this.stageController.pushScene("socket", new socket());
}

