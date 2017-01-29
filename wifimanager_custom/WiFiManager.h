/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <memory>

extern "C" {
  #include "user_interface.h"
}

const char HTTP_HEAD[] PROGMEM            = "       <!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "   <style>a{color:#4b89bf;}.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#4b89bf;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "   <script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}function loading() { document.getElementById('loading').style.display = 'block'; document.getElementById('main').style.display = 'none';}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "   </head><body><div id='loading' style='display:none; margin-top: 125px; '><img src='data:image/png;base64, iVBORw0KGgoAAAANSUhEUgAAADkAAAAmCAYAAAB+mDPLAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAYRJREFUeNrsmb9OAkEQxg9jDxUdSk3jvYHnG+gTgB0l2FBb2hiszg58Eo43OBprYmm1PMHxLZkjSDTqfXveCjPJZrOTMMxv/34DtdYwzoLiNn977Ee7jofX9wTdJRHzatRpJvng7O7Zxp8R8e5PgiMwhVRIhVRIhfQRso13LCw5x17VkOdoSVmgiDtF12Xj1LIsK5qAneGJDFdo11A/iSO4hp08tAtxvaANEN/8KaQkE0oydXHdIpGpa0DE7FW2XfHlKbpIVtLaBEkOyElb7gA+sYD0Sn6zvX67og35zIddAcFv/cyZXzqB/AKUse22B6Q/VYhcCjahBRnqhj3XpYoBATVkGBM4NlU8CqmQCvm/IUWxhL5BnjoGzHWslXmpoyfEjueVK55PKpJIdO1hbFffAWlIqThywIWPgJsqpDWMI+Jnie4eoIGgDqWiKGrpqNM0ri+eGRljCyjjcUD+FyIXmDdnch/wIN9J4zugKh6FVEiFVMgyqxBG4ac/9DFVCG1rAQYAQJuY/J/Dx/UAAAAASUVORK5CYII='/><br /><br />VoltSync Device Thinking...</br></br>Note: If the next page does not load correctly, please hit refresh</div><div id='main' style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "   <center style='margin-bottom: 20px;'>Welcome to VoltSync's custom WiFi setup page.<br /><br />Click the button below to get started.<br/><br/><br/><form action=\"/wifi\" method=\"get\"><button onclick='loading();'>Get Started</button></form>";
const char HTTP_HEADIMAGE[] PROGMEM       = "   <center style='margin-bottom: 20px;'><img src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAI4AAAAQCAYAAADAkUc7AAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAABGlJREFUeNrUWt1x2kAQvjC8QwMJ6gA6QKkg6sDyW96M00BEA7H8lLwhVxC5gogKIioIpAKoILnNfErOy96fHIO9MxqMdHfavf1299vDr15ff06UUrl6KO3PT+9r5ZE3H76k+iNlt2s9tzXGjPXHTBhHsse7Gsv6R7rpsUUPnWKlMXXSa5L+48h1K4xPXPsTqD+Nb7AXWacjvUOvtfXsRxGyf4afOludtg3hvIW+RsaDQ8BkElJiLmxYpwit+zEAgPS+QhtVskeJML/wLJeGvDMEPABuJdgYNF9f5NQVu78gcEjgAUBrwRcp7pt6zF1rGeLdP4CLY8Bp20C/dN8525CRXiz3OHsmbOgdRQCetREOJIVv9LwWgHsuUvcETRfdBJ5LwdaK2wmQNhbQKIseIwB81kc/0oH2HH4axcwd4LMUni08c6Xn3YYQECc9bJkKID6LaDsW0OdRosFTCeCZwuFjIzvXgvMyZJOxJ+jqngFX97VxCOMoS9zpPy9M45AGG0s9vGC31zRWPystymxQFnkp4gB7R04TylaoUHlYW57NA3Tq1pD4zDpCj70JHmSFKwE8mcWBlzbuJ8gE2WoWERiFJYvZ9uSBbUPjSymAIYdCodkmYZvTKZLb6jDIII+2wpIFQyO8srzrF7fDQcw5cDZ6bPqIzLMQAo7A8kMYfgk7bLIBFeCBTmQ5D1SJ+3AHPzXBGQeGEb9YMxReEDIF5s6V2yGqJKVzF3lDlqJ5XxnHmqnziuSY0hONLbrErcXWXK+hhADlPNFXrvdYKxH81fqyNYJiJPgpNMP95TiujiVnL82F8lIYpYdHaRsQjVLrf26SXCEKTbkCkbRdBP7W01gskDFsoMkjdMyEtW5Q/mLkEAOaI+Bg8k5oH8eObHNAqVFCNO7VCxV0m5nDyS6yuhJKnbluKuzzfSRoTB0PAm140ow9sJzN8I3IHC14CQO6VP2Aj	IYYYImQ9hmAh9I+6f9WX9f6WjquQ+h5E/Zr+z/sRVlM2ft9ndZWOH6J4m9DiVyCcU/YJlQWUlwyviKRZhc5zgQyS5xpL6z1p6MzgHoqADWWJsHUix+2zU8JcBwfrFinZeNaWxy6mjyHgJZFk2OhvpubMAExvBBqMnfirdB2ftfzd4IhM8vBU+GIQgJnE1gGm9jazQ7kQktHov79FGCW8FOCuwJ4Vyzz2IT2+IaN/Wbx0xFPswGnVMdH0FcBZa27lwkEehJ4KLjuugpL1plGHlo1PX2RqMf9dNGoEwvAk3q6tm5sCRI/7eGn8cBRg33nKLdS22mQv00P2++FyL19ofy6OBMvy7GPYvfEvpOf1n3eM3AoUDgWpQ5g4SJsIJXL0HaQyKeekwmlj/S4e0GAoVT/NuQY4gklt/iu5EGOQ83r2NI69KA3RTpLkbr3SMFVIPrJ6QXSZyKc87Q4zGo8HUgOwp6osJ/9beVi6eku+LNlxF7S+DYCMBXTr4nQZRvQpqcgzN1+1baDRRwYlvBT9+8VTlt/CzAAvlItOF1uLJkAAAAASUVORK5CYII=' /></center>";
const char HTTP_ITEM_START[] PROGMEM	  =	"   <center style='margin-bottom: 20px;'>Select a WiFi network.</center><div><b>Network</b>&nbsp;<span class='q'><b>Strength</b></span></div>";
const char HTTP_ITEM[] PROGMEM            = "   <div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "   <form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='Password'><br/>";
const char HTTP_FORM_END[] PROGMEM        = "   <br/><button type='submit' onclick='loading();'>Connect</button></form>";
const char HTTP_FORM_PARAM[] PROGMEM      = "   <br/><input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_SCAN_LINK[] PROGMEM       = "   <br/><div class=\"c\"><a href=\"/wifi\" onclick='loading();'>Rescan Networks</a></div>";
const char HTTP_SAVED[] PROGMEM           = "   <div><center><b>Attempting a Connection</b><br/><br/>Your LED should signify if you are succesfully connected.<br /><br /> If the device does not connect, turn the device off then back on and try connecting again.<br /><br /><br /> Please connect back to your original WiFi, then finish your device setup at   <a href='http://www.voltsync.tech'>VoltSync.tech</a><br /><br />Thanks for being a beta tester :) !<br /></center></div>";
const char HTTP_END[] PROGMEM             = "   </div></body></html>";
#define WIFI_MANAGER_MAX_PARAMS 10

class WiFiManagerParameter {
  public:
    WiFiManagerParameter(const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    const char *getCustomHTML();
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    int         _length;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    friend class WiFiManager;
};


class WiFiManager
{
  public:
    WiFiManager();

    boolean       autoConnect();
    boolean       autoConnect(char const *apName, char const *apPassword = NULL);

    //if you want to always start the config portal, without trying to connect first
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String        getConfigPortalSSID();

    void          resetSettings();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //usefully for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void          setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void          setAPCallback( void (*func)(WiFiManager*) );
    //called when settings have been changed and connection was successful
    void          setSaveConfigCallback( void (*func)(void) );
    //adds a custom parameter
    void          addParameter(WiFiManagerParameter *p);
    //if this is set, it will exit after config, even if connection is unsucessful.
    void          setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void          setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void          setRemoveDuplicateAPs(boolean removeDuplicates);

  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<ESP8266WebServer> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    void          handleRoot();
    void          handleWifi(boolean scan);
    void          handleWifiSave();
    void          handleInfo();
    void          handleReset();
    void          handleNotFound();
    void          handle204();
    boolean       captivePortal();

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    WiFiManagerParameter* _params[WIFI_MANAGER_MAX_PARAMS];

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
      return false;
    }
};

#endif
