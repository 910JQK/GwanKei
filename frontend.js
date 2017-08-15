const SVG_NS = 'http://www.w3.org/2000/svg';
const U = 10; // unit
const PIECE_WIDTH = 0.9*U;
const PIECE_HEIGHT = 0.7*U;
const PIECE_FONT_SIZE = '4.2';
const PIECE_COLOR = [
    'hsl(18,100%,20%)',
    'hsl(290,100%,25%)',
    'hsl(130,100%,30%)',
    'hsl(233,100%,20%)'
];
const PIECE_TRANSFORM = ['', '', 'rotate(90)', '', 'rotate(-90)'];
const PIECE_TEXT = [
    '炸彈', '#','#','#','#',
    '#','#','#','#','#',
    '#','#','#','#','#',
    '#','#','#','#','#',
    '#','#','#','#','#',
    '#','#','#','#','#',
    '#','軍旗','工兵','排長','連長',
    '營長','團長','旅長','師長','軍長',
    '司令', '地雷', ''
];


var timer;
var route = [];
var cell_data = {};
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
	    return coor(0, 2, y);
	else if( ((y-1)+1)%4+1 == x )
	    return coor(2, 2, y);
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
    cancel_select();
    for(let g of [route_signs, pieces])
	while(g.firstChild)
	    g.removeChild(g.firstChild);
}


function draw_piece(player, group, y, x, lr, piece_id) {
    var coor = get_rendering_coor(group, y, x, lr);
    var empty = false;
    if(piece_id == -1)
	empty = true;
    let cursor = 'default';
    if(mode == 'preparing' && player == perspective)
	cursor = 'pointer';
    else if(mode == 'playing' && player == perspective)
	cursor = 'pointer';
    else
	cursor = 'default';
    var rect_tag = create_tag(
	'rect',
	{
	    x: coor.x,
	    y: coor.y,
	    width: PIECE_WIDTH,
	    height: PIECE_HEIGHT,
	    stroke: (empty)? 'none': 'black',
	    'stroke-width': 0.5,
	    fill: (empty)? 'hsla(0,0%,0%,0)': PIECE_COLOR[player]
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
	    'text-anchor': 'middle',
	    dy: '1.5',
	    'font-family': 'sans',
	    'font-size': PIECE_FONT_SIZE,
	    fill: 'white'
	},
	{
	    cursor: 'inherit'
	}
    );
    if(!empty)
	text_tag.textContent = PIECE_TEXT[piece_id];
    else
	text_tag.textContent = '';
    var g_tag = create_tag(
	'g',
	{
	    transform: `translate(${coor.x},${coor.y}) ${PIECE_TRANSFORM[get_relative_group(group)]} translate(${-coor.x},${-coor.y})`
	},
	{
	    cursor: cursor
	}
    );
    if(!empty)
	g_tag.classList.add('piece');
    else
	g_tag.classList.add('placeholder');
    g_tag.appendChild(rect_tag);
    g_tag.appendChild(text_tag);
    pieces.appendChild(g_tag);
    g_tag.addEventListener('click', function(ev) {
	var cell = coor2cell(group, y, x, lr);
	var player_of_cell = player;
	ev.stopPropagation();
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
			} else {
			    select_cell(cell);
			}
		    }
		} else if(mode == 'playing') {
		    if(selected_cell != cell) {
			if(perspective == player_of_cell) {
			    select_cell(cell);
			} else if(player_of_cell != (perspective+2)%4) {
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
	let text = create_tag(
	    'text',
	    {
		x: rc.x,
		y: rc.y,
		'text-anchor': 'middle',
		dy: 1,
		'font-size': '8',
		fill: 'red',
		transform: `translate(${rc.x},${rc.y}) rotate(${theta}) translate(${-rc.x},${-rc.y})`
	    }
	);
	text.textContent = '\u2192';
	route_signs.appendChild(text);
    }
    if(route_arr.length > 1) {
	let target_cell = route_arr[route_arr.length-1].cell;
	if(cell_data[target_cell].type == 'empty') {
	    let rect = cell_data[target_cell].svg_tag.querySelector('rect');
	    rect.style.stroke = 'red';
	}
    }
}


function select_cell(cell) {
    cancel_select();
    cell_data[cell].svg_tag.classList.add('selected');
    selected_cell = cell;
    update_cursor();
}


function update_cursor() {
    if(mode == 'playing') {
	if(selected_cell != -1) {
	    for(let I of Object.keys(cell_data)) {
		if(Hub.is_movable(selected_cell, I)) {
		    cell_data[I].svg_tag.style.cursor = 'pointer';
		} // is movable
	    } // for cell
	} else {
	    for(let I of Object.keys(cell_data)) {
		if(cell_data[I].player != perspective) {
		    cell_data[I].svg_tag.style.cursor = 'default';
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
    console.log('render');
    cls();
    mode = board.mode;
    cell_data = {};
    perspective = board.perspective;
    route = [];
    for(let i=0; i<board.length; i++) {
	let element = board.at(i);
	if(element.piece == 43) {
	    route.push(element);
	} else if(element) {
	    let c = cell2coor(element.cell);
	    let tag = draw_piece(
		element.player, c.group, c.y, c.x, c.lr, element.piece
	    );
	    cell_data[element.cell] = {
		type: (element.piece != -1)? 'piece': 'empty',
		player: element.player,
		layout_index: element.layout_index,
		svg_tag: tag
	    };
	}
    }
    draw_route(route);
    board.deleteLater();
}


function set_clock(seconds) {
    function two_digits_num(n) {
	if(n < 10)
	    return '0'+n;
	else
	    return ''+n;
    }
    var current = Hub.get_current_player();
    var clocks = board.querySelectorAll('.clock');
    for(let clock of clocks) {
	clock.style.display = 'none';
    }
    clocks[current].style.display = '';    
    clearInterval(timer);
    timer = setInterval(function() {
	if(seconds > 0)
	    seconds--;
	clocks[current]
	    .querySelector('text')
	    .textContent = two_digits_num(seconds);
    }, 1000);
}


function init_clocks() {
    var pos = {x:40,y:55};
    const width = 15;
    const height = 10;
    for(let i=0; i<4; i++) {
	let rect = create_tag(
	    'rect',
	    {
		x: pos.x,
		y: pos.y,
		width: width,
		height: height,
		fill: 'none',
		stroke: 'black',
		'stroke-width': 1
	    },
	    {
		transform: 'translate(-50%, -50%)'
	    }
	);
	let text = create_tag(
	    'text',
	    {
		x: pos.x,
		y: pos.y,
		dy: 3,
		'text-anchor': 'middle',
		'font-size': 10
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

	clocks.appendChild(g_tag);
    }
}


function init() {
    document.body.addEventListener('click', cancel_select);
    init_clocks();
    Hub.render.connect(render);
    Hub.set_clock.connect(set_clock);
}


window.addEventListener('load', init);


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
