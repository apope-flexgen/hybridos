//DEPRECATED??

/* eslint-disable react/prop-types */
import { withStyles } from 'tss-react/mui';
import React from 'react';

/**
 * Renders virtual keypad
 */
class Keypad extends React.PureComponent {
    constructor() {
        super();

        this.keypad = React.createRef();

        this.handleKeyPress = this.handleKeyPress.bind(this);
    }

    /**
     * Adds mousedown event listener
     */
    componentDidMount() {
        this.keypad.current.addEventListener('mousedown', this.detectMouseDown);
    }

    /**
     * Removes mousedown event listener
     */
    componentWillUnmount() {
        this.keypad.current.removeEventListener('mousedown', this.detectMouseDown);
    }

    /**
     * Prevent loss of focus when clicking keypad
     * @param {*} event 
     */
    detectMouseDown = (event) => {
        // Prevents loss of focus
        event.preventDefault();
    }

    /**
     * Sends input to parent
     * @param {*} event 
     */
    handleKeyPress(event) {
        this.props.parentInputHandler(event, true);
    }

    render() {
        const { classes } = this.props;
        return (
            <div className='keypad' ref={this.keypad} style={{backgroundColor: 'red'}}>
                <span className='dismiss-button-wrapper'><span className='dismiss-button-text'></span><span className='dismiss-button-button'><button className='keypad-button dismiss' id='dismiss' value='dismiss' onClick={this.handleKeyPress}>x</button></span></span>
                <button className='keypad-button backspace' value='backspace' onClick={this.handleKeyPress}>⌫</button>
                <button className='keypad-button sign-change' value='sign-change' onClick={this.handleKeyPress}>+/-</button>
                <button className='keypad-button seven' value='7' onClick={this.handleKeyPress}>7</button>
                <button className='keypad-button eight' value='8' onClick={this.handleKeyPress}>8</button>
                <button className='keypad-button nine' value='9' onClick={this.handleKeyPress}>9</button>
                <button className='keypad-button four' value='4' onClick={this.handleKeyPress}>4</button>
                <button className='keypad-button five' value='5' onClick={this.handleKeyPress}>5</button>
                <button className='keypad-button six' value='6' onClick={this.handleKeyPress}>6</button>
                <button className='keypad-button one' value='1' onClick={this.handleKeyPress}>1</button>
                <button className='keypad-button two' value='2' onClick={this.handleKeyPress}>2</button>
                <button className='keypad-button three' value='3' onClick={this.handleKeyPress}>3</button>
                <button className='keypad-button zero' value='0' onClick={this.handleKeyPress}>0</button>
                <button className='keypad-button decimal' value='decimal' onClick={this.handleKeyPress}>.</button>
            </div>
        );
    }
}
export default (Keypad);
