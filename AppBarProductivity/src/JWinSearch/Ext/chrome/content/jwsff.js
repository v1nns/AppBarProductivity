function JFFTabEnumeratorJS()
{
  this.start();
}

JFFTabEnumeratorJS.prototype = 
{
  ffTabEnumerator:null,
  
  start: function() 
  {
    try 
    {
      const cid = "@jorge.vasquez/fftabenumerator;1";
      var cobj = Components.classes[cid].createInstance();
      ffTabEnumerator = cobj.QueryInterface(Components.interfaces.IJFFEnumerator);

      var sel_topic = ffTabEnumerator.getSelectTopic();
      var enum_topic = ffTabEnumerator.getEnumerateTopic();
     
      var observerService = Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService);
      observerService.addObserver(this, sel_topic, false);
      observerService.addObserver(this, enum_topic, false);
      
      window.addEventListener('unload', function() { this.unload(event); }, false );
    }
    catch (err) 
    {
      alert(err);
      return;
    }
  },
  
  unload: function(event)
  {
      var observerService = Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService);
      observerService.removeObserver(this, sel_topic);
      observerService.removeObserver(this, enum_topic);
      window.removeEventListener('unload', function() { this.unload(event); }, false );
      ffTabEnumerator = null;
  },
  
  observe: function(subject, topic, data) 
  {
    if(topic == ffTabEnumerator.getEnumerateTopic())
    {
      var n_tabs = getBrowser().browsers.length;
      
      for (var i = 0; i < n_tabs; i++) 
      {
        var b = gBrowser.getBrowserAtIndex(i);
        ffTabEnumerator.reportTab(i, b.contentDocument.title, b.currentURI.spec);
      }
    }
    else if(topic == ffTabEnumerator.getSelectTopic())
    {
      var tab_index = parseInt(data, 16);    
      if(isNaN(tab_index))
        return;
      gBrowser.mTabContainer.selectedIndex = tab_index;
    }
  }
}
