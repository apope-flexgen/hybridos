import React, { Component } from 'react';
import Card from '@mui/material/Card';
import CardActions from '@mui/material/CardActions';
import CardContent from '@mui/material/CardContent';
import Button from '@mui/material/Button';
import Box from '@mui/material/Box';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';
import { withStyles } from 'tss-react/mui';
import PropTypes from 'prop-types';
import { STYLES_FEATURES } from '../../styles';
import LoadingHOC from '../LoadingHOC';

// username is in props, has user needed for edit

class PasswordExpirationPage extends Component {
    constructor(props) {
        super(props);
        this.state = {
            password: "",
            confirmPassword: "",
        };
    }
    handleChange = e => {
        this.setState({ [e.target.name]: e.target.value });
      };
    submitUser = () => {
        if (this.state.password === this.state.confirmPassword){
            this.props.setLoading(true)
            const postData = {
              username: this.props.username, 
              password: this.state.password, 
              role: this.props.role,
              user_state_crypto: this.props.userStateCrypto,
              mfa: this.props.mfa,
              requiredAuth: this.props.requiredAuth,
            }
            this.props.superAuth(postData, "password_expiration");
        }
    }  
    render () {
      const { classes } = this.props;
      return (
          <>
          <Box display="flex" justifyContent="center" alignItems="center" minHeight="100vh" minWidth="200vh" >
            <Card>
              <CardContent>
              <Typography sx={{ fontSize: 14 }} color="text.secondary" gutterBottom>
                Password Expiration Page
                </Typography>
                <Typography sx={{ fontSize: 14 }} color="text.secondary" gutterBottom>
                  <TextField
                    id="outlined-basic"
                    label="password"
                    variant="outlined"
                    name="password"
                    value={this.state.password}
                    onChange={this.handleChange}
                  />
                  </Typography>
                  
                  <Typography sx={{ fontSize: 14 }} color="text.secondary" gutterBottom>
                  <TextField
                    id="outlined-password-input"
                    label="confirmPassword"
                    variant="outlined"
                    name="confirmPassword"
                    type="password"
                    value={this.state.confirmPassword}
                    onChange={this.handleChange}
                  />
                  </Typography>
                  </CardContent>
                  <br/>
                  <CardActions>
                  <Typography sx={{ fontSize: 14 }} color="text.secondary" gutterBottom>
                  <Button variant="contained" onClick={this.submitUser}>
                    Submit
                  </Button>
                  </Typography>
            </CardActions>
          </Card>
        </Box>
        </>
        );
    }
}

PasswordExpirationPage.propTypes = {
  classes: PropTypes.object,
};
export default withStyles(LoadingHOC(PasswordExpirationPage), STYLES_FEATURES);