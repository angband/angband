var top_rel, bottom_rel, current_li, rel_list, rel_n;

var more_link = document.createElement("button");
more_link.appendChild(document.createTextNode("- more -"));

var more_top = document.createElement("li");
more_top.onclick = function() { adjust_rel_list(0, -1);}
more_top.appendChild(more_link.cloneNode(true));

var more_bottom = document.createElement("li");
more_bottom.onclick = function() { adjust_rel_list(-1, rel_n);}
more_bottom.appendChild(more_link.cloneNode(true));

function adjust_rel_list(top, bottom) {
	if (top != -1) top_rel = top;
	if (bottom != -1) bottom_rel = bottom;

	if (top_rel < 2) {
            top_rel = 0;
            if ((bottom_rel - top_rel) < 9) bottom_rel = top_rel + 9;
        }

        if (bottom_rel > rel_n - 3) {
            bottom_rel = rel_n - 1;
            if ((bottom_rel - top_rel) < 9) {
                top_rel = bottom_rel - 9;
                if (top_rel < 2) top_rel = 0;
            }
        }

	for (var i = 0; i < rel_n; i++) {
		var el = rel_list[i + 1];

		if (i < top_rel || i > bottom_rel) {
			el.style.display = 'none';
		} else {
			el.style.display = '';
		}
	}

	if (top_rel == 0) more_top.style.display = 'none';
	if (bottom_rel == rel_n - 1) more_bottom.style.display = 'none';
}

window.onload = function () {
	rel_list = document.getElementById("releasesnav").getElementsByTagName("li");
	rel_n = rel_list.length;

	for (var i = 0; i < rel_list.length; i++) {
		if (rel_list[i].childNodes[0].nodeType == 3) current_li = i;
	}

	/* Insert -more- */
	rel_list[0].parentNode.insertBefore(more_top, rel_list[0]);
	rel_list[0].parentNode.appendChild(more_bottom);

	adjust_rel_list(current_li - 4, current_li + 4);
}

