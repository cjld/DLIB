// ==UserScript==
// @name         xuetang10X
// @namespace    https://github.com/cjld/
// @version      0.2
// @description  xuetangX 10X speed up [0.5 ~ 10]
// @author       Dun
// @match        http://www.xuetangx.com/*
// @grant        none
// ==/UserScript==

$(document).ready(function () {
s_button = $(".mejs-rate-selector");
var ul;
function ulappend(sp) {
        ul.append('<li style="background-color: rgba(50, 50, 50, 0);"><input type="radio" name="rate" value="'+sp+
                  '" id="' + sp + '"><label for="' + sp + '">' + sp + '倍速</label></li>');
}

if (s_button.length > 0) {
    ul = s_button.find("ul");
    s_button.height(400);
    for (var i=3; i<=10; i++)
        ulappend(i);
    ulappend(0.5);
}
});
