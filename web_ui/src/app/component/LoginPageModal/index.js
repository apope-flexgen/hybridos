import React, { Component } from 'react';
import { Modal } from "@mui/material";
import PasswordExpirationPage from "../../component/PasswordExpirationPage";
import MfaPage from '../../component/MfaPage';

class LoginPageModal extends Component {
    constructor(props) {
        super(props);
        this.state = {
            openModal: this.props.open,
            userName: this.props.modalData.username,
            user_state_crypto: this.props.modalData.user_state_crypto,
            role: this.props.modalData.role,
            requiredAuth: this.props.modalData.requiredAuth,
            mfa: this.props.modalData.mfa,
            secret_key: this.props.modalData.secret_key,
            page: this.props.page,
            loading: false,
            mfa_failed: false,
        };
    }

    superAuth = (postData,queryName) => {
        let uri = `/superauth?superauthtype=${queryName}`
        fetch(uri, {
          method: 'POST',
          credentials: 'include',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(postData)
        })
        .then((response) => response.json())
        .then((result) => {
            try {
                if ('error' in result) {
                    this.setState({mfa_failed: true, loading: false});
                } else {
                    this.setState({mfa_failed: false, loading: true});
                }

                if (result.requiredAuth.length === 0){
                    this.props.successLogin(this.state.userName, this.state.role) 
                } else if (result.requiredAuth.length === 1) {
                    const newPage = result.requiredAuth.pop();
                    this.setState({page: newPage, secret_key: result.secret_key, openModal: true, requiredAuth: []})
                }
                
            } catch (err) { }
        })
        .catch((error) => {
            console.log('API - editUser error: ', error);
        });
    }

    cancel = () => {
        this.setState({openModal: false})
    }

    updateState = () => {
        this.setState({
            openModal: this.props.open,
            userName: this.props.modalData.username,
            user_state_crypto: this.props.modalData.user_state_crypto,
            role: this.props.modalData.role,
            requiredAuth: this.props.modalData.requiredAuth,
            mfa: this.props.modalData.mfa,
            secret_key: this.props.modalData.secret_key,
            page: this.props.page
        });
    }

    render () {
        return (
            <Modal open = {this.state.openModal}>
                <>
                { this.state.page === "password_expired" && <PasswordExpirationPage username={this.state.userName} 
                        role={this.state.role}
                        userStateCrypto={this.state.user_state_crypto} 
                        mfa={this.state.mfa}
                        requiredAuth={this.state.requiredAuth}
                        superAuth={this.superAuth}
                />}
                { this.state.page === "multi_factor_authentication" && <MfaPage username={this.state.userName} 
                         role={this.state.role}
                         userStateCrypto={this.state.user_state_crypto} 
                         mfa={this.state.mfa}
                         requiredAuth={this.state.requiredAuth}
                         secret_key={this.state.secret_key}
                         superAuth={this.superAuth}
                         cancel = {this.cancel}
                         loading={this.state.loading}
                         mfa_failed={this.state.mfa_failed}
                />}
                </>
            </Modal>
        );
    }
}

export default LoginPageModal;
