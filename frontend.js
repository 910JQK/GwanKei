const SVG_NS = 'http://www.w3.org/2000/svg';
const XLINK = 'http://www.w3.org/1999/xlink';
const U = 10; // unit
const PIECE_WIDTH = 0.9*U;
const PIECE_HEIGHT = 0.7*U;
const PIECE_FONT_SIZE = 0.43*U;
const COLOR_CLASS = ['red', 'purple', 'green', 'blue'];
const PIECE_TRANSFORM = ['', '', 'rotate(90)', '', 'rotate(-90)'];
const PIECE_TEXT = [
    '炸弹', '#','#','#','#',
    '#','#','#','#','#',
    '#','#','#','#','#',
    '#','#','#','#','#',
    '#','#','#','#','#',
    '#','#','#','#','#',
    '#','军旗','工兵','排长','连长',
    '营长','团长','旅长','师长','军长',
    '司令', '地雷', ''
];


var timer;
var route = [];
var cell_data = {};
var cell_of = {}; // subscript: EID
var piece_mark = {};
var mode = 'preparing';
var perspective = 0;
var selected_cell = -1;


class InvalidArgumentException extends Error {};


function invalid_argument() {
    return new InvalidArgumentException();
}


function check_integer(...args) {
    for(let I of args) {
	if(!Number.isInteger(I))
	    throw invalid_argument();
    }
}


function create_tag(name, attrs, style) {
    var tag = document.createElementNS(SVG_NS, name);
    if(attrs) {
	for(let I of Object.keys(attrs))
	    tag.setAttribute(I, attrs[I]);
    }
    if(style) {
	let style_arr = [];
	for(let I of Object.keys(style))
	    style_arr.push(`${I}: ${style[I]}`);
	tag.setAttribute('style', style_arr.join('; '));
    }
    return tag;
}


function get_relative_group(group) {
    if(group != 0) {
	return ((group-1)-perspective+4)%4 + 1;
    } else {
	return 0;
    }
}


function get_rendering_coor(group, y, x, lr) {
    check_integer(group, y, x, lr);
    group = get_relative_group(group);
    function coor(a, b, rotate = 1) {
	if(rotate == 1)
	    return {x:a*U, y:b*U};
	else if(rotate == 2)
	    return {x:b*U, y:-a*U};
	else if(rotate == 3)
	    return {x:-a*U, y:-b*U};
	else if(rotate == 4)
	    return {x:-b*U, y:a*U};
	else
	    throw invalid_argument();
    }
    if(group == 0) {
	if(y == 0 && x == 0)
	    return coor(0, 0);
	else if(x == y && 1 <= y && y <= 4)
	    return coor(0, 2, get_relative_group(y));
	else if( ((y-1)+1)%4+1 == x )
	    return coor(2, 2, get_relative_group(y));
	else
	    throw invalid_argument();
    } else {
	if(1 <= x && x <= 3 && 1 <= y && y <= 6)
	    return coor((lr?1:(-1))*(3-x), 3.5+(y-1), group);
	else
	    throw invalid_argument();
    }
}


function cell2coor(id) {
    return {
	group: Math.floor(id/1000),
	y: Math.floor((id%1000)/100),
	x: Math.floor((id%100)/10),
	lr: Math.floor(id%10)
    };
}


function coor2cell(group, y, x, lr) {
    return group*1000 + y*100 + x*10 + lr;
}


function cls() {
    cls_clocks();
    cancel_select();
    for(let g of [route_signs, pieces])
	while(g.firstChild)
	    g.removeChild(g.firstChild);
}


function draw_piece(player, group, y, x, lr, piece_id, eid) {
    var coor = get_rendering_coor(group, y, x, lr);
    var empty = false;
    if(piece_id == -1)
	empty = true;
    let selectable = (
	(mode == 'playing' || mode == 'preparing')
	    && player == perspective
    );
    var rect_tag = create_tag(
	'rect',
	{
	    x: coor.x,
	    y: coor.y,
	    width: PIECE_WIDTH,
	    height: PIECE_HEIGHT,
	},
	{
	    transform: 'translate(-50%,-50%)'
	}
    );
    var text_tag = create_tag(
	'text',
	{
	    x: coor.x,
	    y: coor.y,
	    dy: '1.5',
	    'font-size': PIECE_FONT_SIZE
	}
    );
    if(!empty) {
	if(piece_mark[eid]) {
	    text_tag.textContent = piece_mark[eid];
	} else {
	    text_tag.textContent = PIECE_TEXT[piece_id];
	}
    } else {
	text_tag.textContent = '';
    }
    var g_tag = create_tag(
	'g',
	{
	    transform: `translate(${coor.x},${coor.y}) ${PIECE_TRANSFORM[get_relative_group(group)]} translate(${-coor.x},${-coor.y})`
	}
    );
    g_tag.classList.add('element');
    if(!empty) {
	g_tag.classList.add('piece');
	g_tag.classList.add(COLOR_CLASS[player]);
    } else {
	g_tag.classList.add('placeholder');
    }
    if(selectable) {
	g_tag.classList.add('selectable');
    }
    g_tag.appendChild(rect_tag);
    g_tag.appendChild(text_tag);
    pieces.appendChild(g_tag);
    g_tag.addEventListener('click', function(ev) {
	var cell = coor2cell(group, y, x, lr);
	var player_of_cell = player;
	ev.stopPropagation();
	hide_panel();
	if(selected_cell != -1) {
	    if(!empty) {
		if(mode == 'preparing') {
		    if(selected_cell != cell) {
			let args = [
			    cell_data[selected_cell].layout_index,
			    cell_data[cell].layout_index
			];
			if(Hub.is_layout_able_to_swap(args[0], args[1])) {
			    Hub.layout_swap(args[0], args[1]);
			    Hub.play_sound('select');
			} else {
			    select_cell(cell);
			}
		    }
		} else if(mode == 'playing') {
		    if(selected_cell != cell) {
			if(perspective == player_of_cell) {
			    select_cell(cell);
			} else if(Hub.is_movable(selected_cell, cell)) {
			    Hub.submit_move(selected_cell, cell);
			}
		    }
		}
	    } else {
		if(mode == 'playing') {
		    Hub.submit_move(selected_cell, cell);
		}
	    }
	} else {
	    if(mode == 'preparing') {
		select_cell(cell);
	    } else if(mode == 'playing') {
		if(perspective == player_of_cell) {
		    select_cell(cell);
		}
	    }
	}
	console.log(`CLICK ${cell}`);
    });
    if(mode != 'preparing' && piece_id == 42) {
	g_tag.addEventListener('contextmenu', function(ev) {
	    ev.preventDefault();
	    show_panel(ev.clientX, ev.clientY);
	    panel.bound_eid = eid;
	});
    }
    return g_tag;
}


function draw_route(route_arr) {    
    function get_rc(element) {
	let c = cell2coor(element.cell);
	return get_rendering_coor(c.group, c.y, c.x, c.lr);
    }
    console.log('-------------');
    for(let i=0; i<route_arr.length-1; i++) {	
	let rc = get_rc(route_arr[i]);
	let next_rc = get_rc(route_arr[i+1]);
	let vector = {x: (next_rc.x - rc.x), y: (next_rc.y - rc.y)};
	let theta = Math.round(Math.atan2(vector.y, vector.x)*180/Math.PI);
	console.log(`[${route_arr[i].cell}] (${rc.x},${rc.y}), ${theta}`);
	let arrow = create_tag(
	    'use',
	    {
		x: rc.x,
		y: rc.y,
		width: 8,
		height: 8,
		transform: `translate(${rc.x}, ${rc.y}) rotate(${theta}) translate(${-rc.x}, ${-rc.y})`
	    }
	);
	arrow.setAttributeNS(XLINK, 'href', '#arrow_path');
	route_signs.appendChild(arrow);
    }
    if(route_arr.length > 1) {
	let target_cell = route_arr[route_arr.length-1].cell;
	cell_data[target_cell].svg_tag.classList.add('target');
    }
}


function select_cell(cell) {
    cancel_select();
    cell_data[cell].svg_tag.classList.add('selected');
    selected_cell = cell;
    update_cursor();
    Hub.play_sound('select');
}


function update_cursor() {
    if(mode == 'playing') {
	if(selected_cell != -1) {
	    for(let I of Object.keys(cell_data)) {
		if(Hub.is_movable(selected_cell, I)) {
		    cell_data[I].svg_tag.classList.add('reachable');
		} // is movable
	    } // for cell
	} else {
	    for(let I of Object.keys(cell_data)) {
		if(cell_data[I].player != perspective) {
		    if(cell_data[I].svg_tag.classList.contains('reachable')) {
			cell_data[I].svg_tag.classList.remove('reachable');
		    }
		}
	    }
	} // selected ?
    } // playing
}


function cancel_select() {
    if(selected_cell != -1) {
	cell_data[selected_cell].svg_tag.classList.remove('selected');
	selected_cell = -1;
    }
    update_cursor();
}


function render(board) {
    /**
       RenderElement(QVariantMap) board::at(int index)
       
       RenderElement {
           "cell": integer, cell id
	   "piece": integer, piece id
               -1 -> empty (special)
               42 -> unknown (ordinary)
               43 -> route (special)
               others -> known piece (ordinary)
	   "player": integer, player id (0..3)
               -1 -> not an ordinary piece, no player
	   "layout_index": integer, layout index (0..24)
               -1 -> not an ordinary piece, no layout index
       }
    */
    console.log('render');
    cls();
    mode = board.mode;
    cell_data = {};
    cell_of = {};
    perspective = board.perspective;
    route = [];
    if(mode == 'preparing') {
	piece_mark = {};
    }
    for(let i=0; i<board.length; i++) {
	let element = board.at(i);
	let eid = element.player*26 + element.layout_index;
	if(element.piece == 43) {
	    route.push(element);
	} else if(element) {
	    let c = cell2coor(element.cell);
	    let tag = draw_piece(
		element.player, c.group, c.y, c.x, c.lr, element.piece, eid
	    );
	    cell_data[element.cell] = {
		type: (element.piece != -1)? 'piece': 'empty',
		player: element.player,
		layout_index: element.layout_index,
		eid: eid,
		svg_tag: tag
	    };
	    cell_of[eid] = element.cell;
	}
    }
    draw_route(route);
    if(mode == 'preparing') {
	ready_button.style.display = '';
    } else {
	ready_button.style.display = 'none';
    }
    board.deleteLater();
}


function cls_clocks() {
    var clocks = document.querySelectorAll('.clock');
    for(let clock of clocks) {
	clock.style.display = 'none';
    }
}


function set_clock(seconds) {
    function two_digits_num(n) {
	if(n < 10)
	    return '0'+n;
	else
	    return ''+n;
    }
    var current = Hub.get_current_player();
    var clock_index = (current-perspective+4) % 4;
    var clocks = document.querySelectorAll('.clock');
    cls_clocks();
    clocks[clock_index].style.display = '';
    clocks[clock_index]
	.querySelector('text')
	.textContent = two_digits_num(seconds);
    clearInterval(timer);
    timer = setInterval(function() {
	if(seconds > 0)
	    seconds--;
	clocks[clock_index]
	    .querySelector('text')
	    .textContent = two_digits_num(seconds);
    }, 1000);
}


function init_clocks() {
    var pos = {x:40,y:55};
    const width = 1.5*U;
    const height = 1*U;
    for(let i=0; i<4; i++) {
	let rect = create_tag(
	    'rect',
	    {
		x: pos.x,
		y: pos.y,
		width: width,
		height: height
	    }
	);
	let text = create_tag(
	    'text',
	    {
		x: pos.x,
		y: pos.y,
		dy: 3,
		'font-size': 1*U
	    }
	);
	text.textContent = '00';
	let g_tag = create_tag('g');
	g_tag.classList.add('clock');
	g_tag.style.display = 'none';
	g_tag.appendChild(rect);
	g_tag.appendChild(text);
	
	let t = pos.x;
	pos.x = pos.y;
	pos.y = -t;

	clock_container.appendChild(g_tag);
    }
}


function show_panel(x, y) {
    panel.style.left = x + 'px';
    panel.style.top = y + 'px';
    panel.style.display = '';
}


function hide_panel() {
    panel.style.display = 'none';
}


function panel_clicked(ev) {
    console.log(ev.target);
    var eid = this.bound_eid;
    var g_tag = cell_data[cell_of[eid]].svg_tag;
    if(ev.target.classList.contains('mark_button')) {
	let mark = ev.target.textContent;
	piece_mark[eid] = mark;
	g_tag.querySelector('text').textContent = mark;
    } else if(ev.target.classList.contains('clear_button')) {
	delete piece_mark[eid];
	g_tag.querySelector('text').textContent = '';	
    }
}


function init() {
    document.body.addEventListener('click', cancel_select);
    document.body.addEventListener('click', hide_panel);
    panel.addEventListener('click', panel_clicked);
    init_clocks();
    Hub.render.connect(render);
    Hub.set_clock.connect(set_clock);
    ready_button.addEventListener('click', () => Hub.submit_ready());
}


window.addEventListener('load', init);


// polyfill
if (NodeList.prototype[Symbol.iterator] === undefined) {
    NodeList.prototype[Symbol.iterator] = function () {
        var i = 0;
        return {
            next: () => {
                return { done: i >= this.length, value: this.item(i++) };
            }
        };
    };
}
