//TODO: Deprecated
import React, { Component } from 'react';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';
import Radio from '@mui/material/Radio';
import FormControlLabel from '@mui/material/FormControlLabel';
import Keyboard from '../Keyboard/index';
import {
    userRole, createUser, removeUser, userUsername,
} from '../../../AppAuth';

/**
 * Component for rendering user administration page
 */
class UserAdministration extends Component {
    constructor() {
        super();
        this.state = {
            username: '',
            password: '',
            role: 'user',
            showKeyboard: true,
            focusedInputRef: null,
            inputIsSelected: false,
        };
        // Input refs
        this.usernameInput = React.createRef();
        this.passwordInput = React.createRef();

        // Input handlers
        this.handleTextFields = this.handleTextFields.bind(this);
        this.handleKeyDown = this.handleKeyDown.bind(this);
        this.handleInputFocus = this.handleInputFocus.bind(this);
        this.handleRadioButton = this.handleRadioButton.bind(this);

        // Keyboard and Mouse listeners
        this.detectPhysicalKeyboard = this.detectPhysicalKeyboard.bind(this);
        this.detectMouseDown = this.detectMouseDown.bind(this);
    }

    // Attach Keyboard and Mouse listener
    /**
     * Attaches keyboard and mouse listeners
     */
    componentDidMount() {
        document.addEventListener('keydown', this.detectPhysicalKeyboard);
        document.addEventListener('mousedown', this.detectMouseDown);
    }

    // Detach Keyboard and Mouse listener
    /**
     * Detaches keyboard and mouse listeners
     */
    componentWillUnmount() {
        document.removeEventListener('keydown', this.detectPhysicalKeyboard);
        document.removeEventListener('mousedown', this.detectMouseDown);
    }

    // Text input handler. Handles both physical and virtual keyboard
    /**
     * Text input handler, handles both physical and virtual keyboards
     * @param {*} event typing event
     * @param {boolean} fromVirtualKeyboard true if event originated from virtual keyboard
     * @param {string} location which input event is targeting
     */
    handleTextFields(event, fromVirtualKeyboard, location) {
        // Prevents error when not focused on a field. Shouldn't be needed with autofocus
        if (!this.state.focusedInputRef) return;

        const { focusedInputRef } = this.state;
        const focusedInput = focusedInputRef.current.id;

        const { value } = event.target;

        if (fromVirtualKeyboard) {
            const { selectionStart, selectionEnd } = focusedInputRef.current;
            const prevValue = this.state[focusedInput];

            switch (value) {
                case 'delete':
                    if (this.state.inputIsSelected) {
                        // Delete whole input after tab
                        this.setState({ [focusedInput]: '', inputIsSelected: false });
                    } else if (selectionStart === selectionEnd) {
                        // Cursor location change
                        if (selectionStart !== 0) {
                            this.setState({
                                [focusedInput]:
                                prevValue.slice(0, selectionStart - 1)
                                + prevValue.slice(selectionStart),
                            }, () => {
                                focusedInputRef.current.selectionStart = selectionStart - 1;
                                focusedInputRef.current.selectionEnd = selectionStart - 1;
                            });
                        }
                    } else {
                        // Selection is highlighted
                        this.setState({
                            [focusedInput]:
                            prevValue.slice(0, selectionStart)
                            + prevValue.slice(selectionEnd),
                        }, () => {
                            focusedInputRef.current.selectionStart = selectionStart;
                            focusedInputRef.current.selectionEnd = selectionStart;
                        });
                    }
                    break;
                case 'tab':
                    // Switches input
                    if (focusedInput === 'username') this.passwordInput.current.select();
                    if (focusedInput === 'password') this.usernameInput.current.select();
                    this.setState({ inputIsSelected: true });
                    break;
                case 'return':
                    if (focusedInput === 'username') this.passwordInput.current.select();
                    break;
                default:
                    if (this.state.inputIsSelected) {
                        // Delete input then add letter after a tab. Mimics physical keyboard
                        this.setState({
                            [focusedInput]: ''.concat(value), inputIsSelected: false,
                        });
                    } else {
                        // Handles both selection and single letter input
                        this.setState({
                            [focusedInput]:
                            prevValue.slice(0, selectionStart)
                            + value
                            + prevValue.slice(selectionEnd),
                        }, () => {
                            focusedInputRef.current.selectionStart = selectionStart + 1;
                            focusedInputRef.current.selectionEnd = selectionStart + 1;
                        });
                    }
            }
        } else {
            // Physical Keyboard
            if (location === 'username') this.setState({ username: value });
            if (location === 'password') this.setState({ password: value });
        }
    }

    // Restricts tab switch to only the inputs. Also lets enter/return key submit login
    /**
     * Prevents default event handling for tab and enter keys
     * @param {*} event event for key press
     */
    handleKeyDown(event) {
        if (this.state.focusedInputRef.current.id === 'username') {
            if (event.shiftKey && event.key === 'Tab') {
                event.preventDefault();
                this.passwordInput.current.select();
            }
        }
        if (this.state.focusedInputRef.current.id === 'password') {
            if (event.key === 'Tab') {
                event.preventDefault();
                this.usernameInput.current.select();
            }
            if (event.key === 'Enter') {
                event.preventDefault();
                this.doLoginWithState();
            }
        }
    }

    // Keeps focused input in state to determine where virtual keyboard is sending values
    /**
     * Sets focused input in state to determine where virtual keyboard is sending values
     * @param {*} event location of focus event
     */
    handleInputFocus(event) {
        let focusedInputRef;
        if (event.target.id === 'username') focusedInputRef = this.usernameInput;
        if (event.target.id === 'password') focusedInputRef = this.passwordInput;
        this.setState({ focusedInputRef });
    }

    // Checks if physical keyboard is used
    /**
     * Checks if user inputs with physical keyboard
     * @param {*} event physical keyboard event
     */
    detectPhysicalKeyboard(event) {
        if (event) this.setState({ showKeyboard: false });
    }

    // Used to prevent virtual keyboard buttons from removing focus
    /**
     * Prevents mouse events on virtual keyboard from removing focus
     * @param {*} event mouse down event
     */
    detectMouseDown(event) {
        this.setState({ inputIsSelected: false });

        // Prevents loss of focus
        if (event.target.tagName !== 'INPUT') event.preventDefault();

        // Checks if mouse clicks a radio button, prevents loss of focus
        if (event.target.tagName === 'INPUT' && event.target.type === 'radio') event.preventDefault();
    }

    // Sets radio button focus
    /**
     * Sets focus for radio button
     * @param {*} event
     */
    handleRadioButton(event) {
        this.setState({ role: event.target.value });
    }

    // Submits state
    /**
     * Submits state
     * @param {*} event
     * @param {string} location type of submit [create, remove]
     */
    handleSubmit(event, location) {
        if (location === 'create') createUser(this.state.username, this.state.password, this.state.role);
        if (location === 'remove') removeUser(this.state.username);
    }

    render() {
        const theRoles = ['user', 'admin', 'rest', 'observer'];
        if (userRole === 'developer' || userUsername === 'iamaflexgendeveloper') theRoles.push('developer');
        // TODO: (not a to-do, but an important note)
        // in case of emergency, if there are no known developer accounts on a site, you can
        // create an admin account with the name "iamaflexgendeveloper", login as
        // "iamaflexgendeveloper" and then you will see the developer role option. Generally
        // speaking, we do not want our customers to have access to the developer role options.
        return (
            <>
                {/* eslint-disable max-len */}
                <div className='userAdministration-help'><em>Tips for using User Administration</em><br /><br />
                    <strong>&bull; usernames</strong>: Username must be at least 5 characters long, and no more than 25 characters long. It may only include letters, numbers, periods, and underscores. All letters (a-z, A-Z) will be converted to lower case but may be mixed however the user wishes.<br />
                    <strong>&bull; passwords</strong>: Password must be at least 5 characters long, and no more than 25 characters long. It must include uppercase and lowercase letters, at least one number, and at least one of these special characters: !&quo;#$%&amp;&apos;*+,./:;=\?@^`|~<br />
                    <strong>&bull; roles</strong>: Generally speaking, give a user the minimum role they can work with. Most users do not need admin access unless you need for them to be able to create and remove other users. The &ldquo;rest&rdquo; role is only used for external REST API access. Please do not give a user &ldquo;rest&rdquo; access unless you are certain that is what you want to do.<br /><br />
                    <strong>&bull; Create User</strong>: Enter a username, password, and role, then click &ldquo;Create User&rdquo;.<br />
                    <strong>&bull; Remove User</strong>: Enter a username, then click &ldquo;Remove User&rdquo;.<br /><br />
                    <strong>&bull; How to Change a Password</strong>: Because user preferences are not saved in this application, the way to change a password is simply to <em>Remove User</em> and then <em>Create User</em> with a new password.</div>
                {/* eslint-enable max-len */}
                <form autoComplete='off' className='userAdministration-form'>
                    <div className='userAdministration-TextField-wrapper'>
                        <TextField
                            value={this.state.username}
                            onChange={(event) => this.handleTextFields(event, false, 'username')}
                            onKeyDown={this.handleKeyDown}
                            onFocus={this.handleInputFocus}
                            id='username'
                            label="Username"
                            type='search' // <-- this makes LastPass ignore this field
                            className='userAdministration-input'
                            inputRef={this.usernameInput}
                            style={{ width: '100%' }}
                            autoFocus
                        />
                        <TextField
                            value={this.state.password}
                            onChange={(event) => this.handleTextFields(event, false, 'password')}
                            onKeyDown={this.handleKeyDown}
                            onFocus={this.handleInputFocus}
                            id='password'
                            label="Password"
                            type='search' // <-- this makes LastPass ignore this field
                            className='userAdministration-input'
                            inputRef={this.passwordInput}
                            style={{ width: '100%', marginTop: '2%' }}
                        />
                        <br />
                        {theRoles.map((role, i) => (
                            <FormControlLabel
                                key={i}
                                control={
                                    <Radio
                                        color='primary'
                                        checked={this.state.role === role}
                                        onChange={this.handleRadioButton}
                                        value={role}
                                    />
                                }
                                label={role}
                                labelPlacement='end'
                            />
                        ))}
                        <br />
                        <br />
                    </div>
                    <div className='userAdministration-Button-wrapper'>
                        <Button
                            style={{ left: 0 }}
                            id='createUser-button'
                            variant='contained'
                            color='secondary' // this is the bg color of the button
                            onClick={(event) => this.handleSubmit(event, 'create')}
                        >
                            {'Create User'}
                        </Button>
                        <Button
                            style={{ position: 'absolute', right: '0' }}
                            id='removeUser-button'
                            variant='contained'
                            color='primary' // this is the bg color of the button
                            onClick={(event) => this.handleSubmit(event, 'remove')}
                        >
                            {'Remove User'}
                        </Button>
                    </div>
                </form>
                <div className='userAdministration-console-wrapper' id='consoleDisplay' overflow='auto' style={{
                    fontFamily: '"Courier New", Courier, monospace', fontSize: '0.8em', width: '50%', height: '4vh', textAlign: 'left',
                }} note='these style items must be in inline styling, some CSS styling methods do not work in React'></div>
                <br />
                <div className='userAdministration-Keyboard-wrapper'>
                    <Keyboard parentInputHandler={this.handleTextFields} />
                </div>
            </>
        );
    }
}

export default UserAdministration;
