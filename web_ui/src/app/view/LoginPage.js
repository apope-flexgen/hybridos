/* eslint-disable react/prop-types */
import React, { Component } from 'react';
import AppBar from '@mui/material/AppBar';
import Toolbar from '@mui/material/Toolbar';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';
import { Typography } from '@mui/material';

import Keyboard from "../component/Keyboard/index";

import { FLEXGEN_LOGO } from "../../AppConfig";
import { doLogin, superAuthLogin } from "../../AppAuth";
import LoginPageModal from "../component/LoginPageModal";
import { withStyles } from "tss-react/mui";
import PropTypes from "prop-types";
import { STYLES_LOGIN_PAGE } from "../styles";
import LoadingHOC from '../component/LoadingHOC';

/**
 * Component for rendering login page
 */
class LoginPage extends Component {
    constructor(props) {
        super(props);
        this.state = {
            username: '',
            password: '',
            showKeyboard: true,
            focusedInputRef: null,
            inputIsSelected: false,
            openModal: false,
            modalData: {},
            page: "",
        };
        this.props.setLoading(false)
        // Input refs
        this.usernameInput = React.createRef();
        this.passwordInput = React.createRef();

        // Login model ref
        this.loginModalRef = React.createRef();

        // Input handlers
        this.handleTextFields = this.handleTextFields.bind(this);
        this.handleKeyDown = this.handleKeyDown.bind(this);
        this.handleInputFocus = this.handleInputFocus.bind(this);

        // Keyboard and Mouse listeners
        this.detectPhysicalKeyboard = this.detectPhysicalKeyboard.bind(this);
        this.detectMouseDown = this.detectMouseDown.bind(this);

        // Login Handler
        this.doLoginWithState = this.doLoginWithState.bind(this);
    }

    // Attach Keyboard and Mouse listener
    componentDidMount() {
        document.addEventListener("keydown", this.detectPhysicalKeyboard);
        document.addEventListener("mousedown", this.detectMouseDown);
    }

    // Detach Keyboard and Mouse listener
    componentWillUnmount() {
        document.removeEventListener("keydown", this.detectPhysicalKeyboard);
        document.removeEventListener("mousedown", this.detectMouseDown);
    }

    // Text input handler. Handles both physical and virtual keyboard
    /**
     * Text input handler for virtual and physical keyboard
     * @param {*} event type of input
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
                case "delete":
                    if (this.state.inputIsSelected) {
                        // Delete whole input after tab
                        this.setState({
                            [focusedInput]: "",
                            inputIsSelected: false,
                        });
                    } else if (selectionStart === selectionEnd) {
                        // Cursor location change
                        if (selectionStart !== 0) {
                            this.setState(
                                {
                                    [focusedInput]:
                                        prevValue.slice(0, selectionStart - 1) +
                                        prevValue.slice(selectionStart),
                                },
                                () => {
                                    focusedInputRef.current.selectionStart =
                                        selectionStart - 1;
                                    focusedInputRef.current.selectionEnd =
                                        selectionStart - 1;
                                }
                            );
                        }
                    } else {
                        // Selection is highlighted
                        this.setState(
                            {
                                [focusedInput]:
                                    prevValue.slice(0, selectionStart) +
                                    prevValue.slice(selectionEnd),
                            },
                            () => {
                                focusedInputRef.current.selectionStart = selectionStart;
                                focusedInputRef.current.selectionEnd = selectionStart;
                            }
                        );
                    }
                    break;
                case "tab":
                    // Switches input
                    if (focusedInput === "username")
                        this.passwordInput.current.select();
                    if (focusedInput === "password")
                        this.usernameInput.current.select();
                    this.setState({ inputIsSelected: true });
                    break;
                case "return":
                    if (focusedInput === "username") {
                        if (this.state.password) this.doLoginWithState();
                        else this.passwordInput.current.select();
                    }
                    if (focusedInput === "password") this.doLoginWithState();
                    break;
                default:
                    if (this.state.inputIsSelected) {
                        // Delete input then add letter after a tab. Mimics physical keyboard
                        this.setState({
                            [focusedInput]: "".concat(value),
                            inputIsSelected: false,
                        });
                    } else {
                        // Handles both selection and single letter input
                        this.setState(
                            {
                                [focusedInput]:
                                    prevValue.slice(0, selectionStart) +
                                    value +
                                    prevValue.slice(selectionEnd),
                            },
                            () => {
                                focusedInputRef.current.selectionStart =
                                    selectionStart + 1;
                                focusedInputRef.current.selectionEnd =
                                    selectionStart + 1;
                            }
                        );
                    }
            }
        } else {
            // Physical Keyboard
            if (location === "username") this.setState({ username: value });
            if (location === "password") this.setState({ password: value });
        }
    }

    // Restricts tab switch to only the inputs. Also lets enter/return key submit login
    /**
     * Prevents default event handling for tab and enter keys
     * @param {*} event event for key press
     */
    handleKeyDown(event) {
        if (this.state.focusedInputRef.current.id === "username") {
            if (event.shiftKey && event.key === "Tab") {
                event.preventDefault();
                this.passwordInput.current.select();
            }
            if (event.key === "Enter") {
                event.preventDefault();
                this.doLoginWithState();
            }
        }
        if (this.state.focusedInputRef.current.id === "password") {
            if (event.key === "Tab") {
                event.preventDefault();
                this.usernameInput.current.select();
            }
            if (event.key === "Enter") {
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
        if (event.target.id === "username")
            focusedInputRef = this.usernameInput;
        if (event.target.id === "password")
            focusedInputRef = this.passwordInput;
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
        if (event.target.tagName !== "INPUT") event.preventDefault();
    }
    // finishes logging in a user who has used password exp or mfa login
    successLogin = (username, role) => {
        superAuthLogin(username, role, () => {
            this.props.setStateFromChild("isLoggedIn", true);
        });
    };

    // Login
    /**
     * Logs in
     */
    doLoginWithState() {
        this.props.setLoading(true)
        doLogin(this.state.username, this.state.password, (response) => {
            if (response.message==="password_expired"){
                this.props.setLoading(false)
                this.setState({
                    page: "password_expired",
                    openModal: true,
                    modalData: {
                        username: this.state.username,
                        role: response.role,
                        user_state_crypto: response.user_state_crypto,
                        mfa: response.mfa,
                        requiredAuth: response.requiredAuth,
                    }
                }, () => this.loginModalRef.current.updateState());
            } else if (response.message==="multi_factor_authentication"){
                this.props.setLoading(false)
                this.setState({
                    page: "multi_factor_authentication",
                    openModal: true,
                    modalData: {
                        username: this.state.username,
                        role: response.role,
                        user_state_crypto: response.user_state_crypto,
                        mfa: response.mfa,
                        secret_key: response.secret_key,
                        requiredAuth: response.requiredAuth,
                    },
                }, () => this.loginModalRef.current.updateState());
            } else {
                this.props.setStateFromChild("isLoggedIn", true);
            }
        });
    }

    render() {
        const { classes } = this.props;

        return (
            <>
                <AppBar position="absolute">
                    <Toolbar disableGutters={false}>
                        <span className={classes.toolbarText}>
                            <Typography
                                noWrap
                                className={classes.toolbarText}
                                id="login-title"
                            >
                                Login
                            </Typography>
                        </span>
                        <img
                            src={FLEXGEN_LOGO}
                            alt="FlexGen"
                            className={classes.logo}
                        />
                    </Toolbar>
                </AppBar>
                {<form autoComplete='off' className={classes.form}>
                        <div className={classes.wrapper}>
                            <TextField
                                value={this.state.username}
                                onChange={(event) =>
                                    this.handleTextFields(
                                        event,
                                        false,
                                        "username"
                                    )
                                }
                                onKeyDown={this.handleKeyDown}
                                onFocus={this.handleInputFocus}
                                id="username"
                                type="text"
                                label="Username"
                                inputRef={this.usernameInput}
                                autoFocus
                            />
                            <br />
                            <br />
                            <TextField
                                value={this.state.password}
                                onChange={(event) =>
                                    this.handleTextFields(
                                        event,
                                        false,
                                        "password"
                                    )
                                }
                                onKeyDown={this.handleKeyDown}
                                onFocus={this.handleInputFocus}
                                id="password"
                                type="password"
                                label="Password"
                                inputRef={this.passwordInput}
                            />
                            <br />
                            <br />
                            <Button
                                style={{ margin: "1% 5% 5% 5%" }}
                                id="login-button"
                                variant="contained"
                                color="primary" // this is the bg color of the button
                                onClick={this.doLoginWithState}
                            >
                                Login
                            </Button>
                        </div>
                        <LoginPageModal
                            open={this.state.openModal}
                            page={this.state.page}
                            modalData={this.state.modalData}
                            successLogin={this.successLogin}
                            ref={this.loginModalRef}
                        />
                        {/* To use both, switch to non-conditional */}
                        {this.state.showKeyboard && (
                            <Keyboard
                                parentInputHandler={this.handleTextFields}
                            />
                        )}
                        {/* <Keyboard parentInputHandler={this.handleTextFields} /> */}
                    </form>
                }
            </>
        );
    }
}

LoginPage.propTypes = {
    classes: PropTypes.object,
};
export default withStyles(LoadingHOC(LoginPage), STYLES_LOGIN_PAGE);
