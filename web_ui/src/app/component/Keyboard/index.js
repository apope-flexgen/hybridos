/* eslint-disable react/prop-types */
import { withStyles } from 'tss-react/mui';
import PropTypes from 'prop-types';
import { STYLES_KEYBOARD } from '../../styles';
import React from 'react';
import classNames from 'classnames';

const keyboardFirstRowNoShift = ['`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '='];
const keyboardSecondRowNoShift = ['q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\'];
const keyboardThirdRowNoShift = ['a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'"];
const keyboardFourthRowNoShift = ['z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'];

const keyboardFirstRowShift = ['~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+'];
const keyboardSecondRowShift = ['Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '|'];
const keyboardThirdRowShift = ['A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"'];
const keyboardFourthRowShift = ['Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'];

/**
 * Render virtual keyboard
 */
class Keyboard extends React.PureComponent {
    constructor() {
        super();
        this.state = {
            shiftKeyOn: false,
            capsLockOn: false,
        };

        this.handleKeyPress = this.handleKeyPress.bind(this);
    }

    /**
     * Handles shift and caps lock logic
     * @param {*} event click event
     */
    handleKeyPress(event) {
        event.preventDefault();

        switch (event.target.value) {
            case 'shift':
                this.setState({ shiftKeyOn: !this.state.shiftKeyOn });
                break;
            case 'capslock':
                this.setState({ capsLockOn: !this.state.capsLockOn });
                break;
            default:
                this.setState({ shiftKeyOn: false });
                this.props.parentInputHandler(event, true);
        }
    }

    render() {
        const shiftKeyColor = this.state.shiftKeyOn ? 'white' : 'black';
        const shiftKeyBackgroundColor = this.state.shiftKeyOn ? 'darkgray' : 'white';

        const capsLockKeyColor = this.state.capsLockOn ? 'white' : 'black';
        const capsLockKeyBackgroundColor = this.state.capsLockOn ? 'darkgray' : 'white';
        
        const { classes } = this.props;

        return (
            <div className={classes.keyboard}>
                {/* keyboard first row */}
                {this.state.shiftKeyOn === this.state.capsLockOn
                    ? keyboardFirstRowNoShift.map((key) => <button className={classNames(classes.element, classes.button)} key={key} value={key} onClick={this.handleKeyPress}>{key}</button>)
                    : keyboardFirstRowShift.map((key) => <button className={classNames(classes.element, classes.button)} key={key} value={key} onClick={this.handleKeyPress}>{key}</button>)}
                <button className={classNames(classes.element, classes.buttonTab)} value='delete' onClick={this.handleKeyPress}>delete</button>

                {/* keyboard second row */}
                <button className={classNames(classes.element, classes.buttonTab)} value='tab' onClick={this.handleKeyPress}>tab</button>
                {this.state.shiftKeyOn === this.state.capsLockOn
                    ? keyboardSecondRowNoShift.map((key) => <button className={classNames(classes.element, classes.button)} key={key} value={key} onClick={this.handleKeyPress}>{key}</button>)
                    : keyboardSecondRowShift.map((key) => <button className={classNames(classes.element, classes.button)} key={key} value={key} onClick={this.handleKeyPress}>{key}</button>)}

                {/* keyboard third row */}
                <button className={classNames(classes.element, classes.buttonCapsLock)} value='capslock' onClick={this.handleKeyPress} style={{ backgroundColor: capsLockKeyBackgroundColor, color: capsLockKeyColor }}>capslock</button>
                {this.state.shiftKeyOn === this.state.capsLockOn
                    ? keyboardThirdRowNoShift.map((key) => <button className={classNames(classes.element, classes.button)} key={key} value={key} onClick={this.handleKeyPress}>{key}</button>)
                    : keyboardThirdRowShift.map((key) => <button className={classNames(classes.element, classes.button)} key={key} value={key} onClick={this.handleKeyPress}>{key}</button>)}
                <button className={classNames(classes.element, classes.buttonReturn)} value='return' onClick={this.handleKeyPress}>return</button>

                {/* keyboard fourth row */}
                <button className={classNames(classes.element, classes.buttonShift)} value='shift' onClick={this.handleKeyPress} style={{ backgroundColor: shiftKeyBackgroundColor, color: shiftKeyColor }}>shift</button>
                {this.state.shiftKeyOn === this.state.capsLockOn
                    ? keyboardFourthRowNoShift.map((key) => <button className={classNames(classes.element, classes.button)} key={key} value={key} onClick={this.handleKeyPress}>{key}</button>)
                    : keyboardFourthRowShift.map((key) => <button className={classNames(classes.element, classes.button)} key={key} value={key} onClick={this.handleKeyPress}>{key}</button>)}
                <button className={classNames(classes.element, classes.buttonShift)} value='shift' onClick={this.handleKeyPress} style={{ backgroundColor: shiftKeyBackgroundColor, color: shiftKeyColor }}>shift</button>
            </div>
        );
    }
}
Keyboard.propTypes = {
    classes: PropTypes.object,
};
export default withStyles(Keyboard, STYLES_KEYBOARD);
