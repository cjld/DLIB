// ==UserScript==
// @name         xuetang10X
// @namespace    https://github.com/cjld/
// @version      0.1
// @description  xuetangX 10X speed up
// @author       Dun
// @match        http://www.xuetangx.com/*
// @grant        none
// ==/UserScript==

$(document).ready(function () {
s_button = $(".mejs-rate-selector");
console.log(s_button);
if (s_button.length > 0) {
    ul = s_button.find("ul");
    s_button.height(350);
console.log(ul);
    for (var i=3; i<=10; i++)
        ul.append('<li style="background-color: rgba(50, 50, 50, 0);"><input type="radio" name="rate" value="'+i+
                  '" id="' + i + '"><label for="' + i + '">' + i + '倍速</label></li>');
}
});