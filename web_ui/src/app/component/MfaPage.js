import React from 'react';
import Card from '@mui/material/Card';
import CardActions from '@mui/material/CardActions';
import CardContent from '@mui/material/CardContent';
import CardMedia from '@mui/material/CardMedia';
import Button from '@mui/material/Button';
import Box from '@mui/material/Box';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';
import LoadingHOC from './LoadingHOC';

class MfaPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
           totp: "",
           qrcodeString: this.props.secret_key,
           mfa: this.props.mfa,
        };
        this.handleKeyDown = this.handleKeyDown.bind(this);
    }
    handleChange = e => {
        this.setState({ [e.target.name]: e.target.value });
    };

    // Lets enter/return key submit login
    /**
     * Prevents default event handling for enter key
     * @param {*} event event for key press
     */
    handleKeyDown(event) {
        if (event.key === 'Enter') {
            event.preventDefault();
            this.submitUser();
        } 
    }

    submitUser = () => {
        this.props.setLoading(true)
        const postData = {
            username: this.props.username, 
            totp: this.state.totp, 
            role: this.props.role,
            user_state_crypto: this.props.userStateCrypto,
            mfa: this.props.mfa,
            requiredAuth: this.props.requiredAuth,
        }
        this.props.superAuth(postData, "multi_factor_authentication");
        
    } 
    exit = () => {
        this.props.cancel();
    }
    render () {
        let qrStatus = (this.props.mfa === 1);

        return (
            <>
            <Box display="flex" justifyContent="center" alignItems="center" minHeight="100vh" minWidth="200vh">
                <Card style={{width:"275px", display:"flex", flexDirection:"column", justifyContent:"center", alignItems:"center"}}>  
                    <CardContent>
                        <Typography sx={{ fontSize: 14 }} color="text.secondary" gutterBottom>
                        Multi-Factor Authentication Page
                        </Typography>
                        <br></br>
                        {qrStatus && <CardMedia component="img" image={this.state.qrcodeString}/>}
                        <Typography sx={{ fontSize: 14 }} color="text.secondary" gutterBottom>
                            <TextField
                            // time based auth token
                            id="outlined-password-input"
                            label="Time Based Token"
                            variant="outlined"
                            name="totp"
                            type="password"
                            error={this.props.mfa_failed}
                            helperText={this.props.mfa_failed ? "Incorrect Code, Try again" : ""}
                            value={this.state.totp}
                            onChange={this.handleChange}
                            exit = {this.exit}
                            onKeyDown={this.handleKeyDown}
                            autoFocus
                            />
                        </Typography>
                    </CardContent>
                    <CardActions>
                        <Typography sx={{ fontSize: 14 }} color="text.secondary" gutterBottom>
                            <Button variant="contained" color="primary" onClick={this.submitUser}>
                                Submit
                            </Button>
                            &nbsp;&nbsp;&nbsp;
                            <Button variant="contained" color="primary" onClick={this.exit}>
                                Cancel
                            </Button>
                        </Typography>
                    </CardActions>
                </Card>
            </Box>
            </>
        );
    }
}

export default LoadingHOC(MfaPage);