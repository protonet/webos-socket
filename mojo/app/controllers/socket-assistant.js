/* Copyright 2010 Protonet.info  All rights reserved. */

var SocketAssistant = Class.create(
{

  setup: function()
  {
    this.stageDocument = this.controller.stageController.document;

    this.controller.setupWidget('ConnectBtn',
    this.attributes = {
    },
    this.model = {
      label: 'Connect',
      disabled: false
    });
    this.controller.setupWidget("ipField",
    this.attributes = {
      hintText: $L("ip address"),
      multiline: false,
      enterSubmits: true,
      focus: true
    },
    this.model = {
      value: "192.168.1.136",
      disabled: false
    });


    Mojo.Event.listen(this.controller.get("ConnectBtn"),Mojo.Event.tap, this.handleConnect);

    $('status').innerHTML = '---';

    $('socketPlugin').didReceiveData = this.didReceiveData.bind(this);
  },

  didReceiveData: function(a)
  {
    $('status').innerHTML = String(a);
  },

  handleConnect: function(event)
  {
    $('socketPlugin').openSocket($('ipField').mojo.getValue(), 2000);
  }

});


