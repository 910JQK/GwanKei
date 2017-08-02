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


function get_coordinate(group, y, x, lr) {
    check_integer(group, y, x, lr);
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


function cls() {
    while(pieces.firstChild)
	pieces.removeChild(pieces.firstChild);
}


function draw_piece(player, group, y, x, lr, piece_id) {
    var coor = get_coordinate(group, y, x, lr);
    var rect_tag = create_tag(
	'rect',
	{
	    x: coor.x,
	    y: coor.y,
	    width: PIECE_WIDTH,
	    height: PIECE_HEIGHT,
	    stroke: 'black',
	    'stroke-width': 0.5,
	    fill: PIECE_COLOR[player]
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
	}
    );
    text_tag.textContent = PIECE_TEXT[piece_id];
    var g_tag = create_tag(
	'g',
	{
	    transform: `translate(${coor.x},${coor.y}) ${PIECE_TRANSFORM[group]} translate(${-coor.x},${-coor.y})`
	}
    );
    g_tag.appendChild(rect_tag);
    g_tag.appendChild(text_tag);
    pieces.appendChild(g_tag);
}


function update_board(board) {
    cls();
    route = [];
    for(let i=0; i<board.length; i++) {
	let element = board.at(i);
	if(element.piece == 43) {
	    route.push(element);
	} else if(element) {
	    let c = cell2coor(element.cell);
	    draw_piece(element.player, c.group, c.y, c.x, c.lr, element.piece);
	}
    }
    //draw_route(route);
}


function init() {
    Hub.update_board.connect(update_board);
}


window.addEventListener('load', init);
