const colors = require('colors');

const customLog = (string, color, decorated = false, decoration = '=') => {
    const spaces = string.length <= 31 ? ' '.repeat((32 - string.length) / 2) : ' ';
    if (decorated) {
        string = decoration.repeat(30) + spaces + string + spaces + decoration.repeat(30);
    }

    switch (color) {
        case 'g':
            console.log(string.green);
            break;
        case 'b':
            console.log(string.blue);
            break;
        case 'r':
            console.log(string.red);
            break;
        case 'y':
            console.log(string.yellow);
            break;
        case 'm':
            console.log(string.magenta);
            break;
        case 'c':
            console.log(string.cyan);
            break;
        default:
            console.log(string);
            break;
    }
}

module.exports = {
    customLog
}