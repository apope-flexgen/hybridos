import React from 'react';
import { withStyles } from 'tss-react/mui';

import {
    Box,
    FormControl,
    InputLabel,
    FormHelperText,
    Select,
    Switch,
    MenuItem,
    Button,
    Paper,
    List,
    ListSubheader,
    ListItem,
    ListItemText,
} from '@mui/material';

import {
    getDataForURI,
    setOrPutDataForURI,
} from '../../AppConfig';

import {
    createUser,
    removeUser,
    editUser,
    userRole
} from '../../AppAuth';

import { Add } from '@mui/icons-material';

import { STYLES_USER_ADMIN } from '../styles';
import { capitalizeFirst } from '../helpers';

import Loading from '../component/Loading';
import SaveCancel from '../component/SaveCancel';
import Confirm from '../component/Confirm';
import { TextInput, ButtonInput } from '../component/Inputs';

import { setPopupAlert } from '../../actions/index';
import { connect } from 'react-redux';

const ROLES = [
    'admin',
    'user',
    'observer',
    'rest'
]

class UserAdministration extends React.Component {
    constructor() {
        super();

        this.state = {
            selectedRole: 'none',
            isLoading: false,
            selectedIndex: null,
            users: [],
            newUsers: [],
            newFields: null,
            readyDelete: false,
            showAlert: false,
            }
    }

    fetchRole = (selectedRole) => {
        this.setState({ isLoading: true, selectedRole });
        getDataForURI(`users/read/summary?role=${selectedRole}`)
            .then((response) => response.json())
            .then((response) => {
                this.setState({
                    isLoading: false,
                    users: response.message,
                    selectedIndex: null
                })
            })
            .catch((error) => {
                throw new Error(`ERROR in USER_ADMINISTRATION/fetchRole: ${error}`);
            });
    }

    handleSelectRole = (event) => {
        if (event.target.value !== 'none') this.fetchRole(event.target.value);
        else {
            this.setState({
                selectedRole: 'none',
                selectedIndex: null,
                users: []
            });
        }
    }

    handleSelectUser = (user, index) => {
        this.setState({
            selectedIndex: this.state.selectedIndex === index ? null : index,
            readyDelete: false,
            newFields: {
                username: user.username,
                role: user.role,
                password: '',
                isNew: user.isNew,
            },
        })
    }

    handleChangeUser = (event, field) => {
        let copy = Object.assign({}, this.state.newFields);
        copy[field] = event.target.value;
        this.setState({ newFields: copy });
    }

    addUser = () => {
        let usersCopy = [...this.state.users];
        usersCopy.push({
            username: 'New User',
            role: this.state.selectedRole,
            isNew: true
        });
        this.setState({
            users: usersCopy
        })
    }

    handleDelete = (event, type) => {
        var removeUser = () => {
            let usersCopy = [...this.state.users];
            usersCopy.splice(this.state.selectedIndex, 1);
            this.setState({ users: usersCopy, selectedIndex: null });
        }
        if (type === 'confirm') {
            if (!this.state.users[this.state.selectedIndex].isNew) {
                // removeUser(this.state.users[this.state.selectedIndex].username);
                fetch(`/api/removeuser?username=${encodeURIComponent(this.state.users[this.state.selectedIndex].username)}`, {
                    method: 'POST',
                    credentials: 'include',
                })
                    .then((response) => response.json())
                    .then((response) => {
                        this.setState({
                            showAlert: true,
                            popupStatus: response.status,
                            popupMessage: response.message
                        })
                        if (response.status === 200) {
                            removeUser();
                        }
                    })
                    .catch((error) => {
                        throw new Error(`ERROR in USER_ADMINISTRATION/removeUser: ${error}`);
                    })
            } else {
                removeUser();
            }
        } else {
            this.setState({ readyDelete: false })
        }
    }

    handleSave = (save) => {
        let user = this.state.users[this.state.selectedIndex];
        if (save) {
            if (user.isNew) {
                const { users, selectedIndex } = this.state;
                const { username, password, role } = this.state.newFields;
                fetch(`/api/createuser?username=${encodeURIComponent(username)}&password=${encodeURIComponent(password)}&role=${encodeURIComponent(role)}`, {
                    method: 'POST',
                    credentials: 'include',
                })
                    .then((response) => response.json())
                    .then((response) => {
                        this.props.dispatch(setPopupAlert(response.status, response.message, true))
                        if (response.status === 200) {
                            let usersCopy = [...users];
                            usersCopy[selectedIndex] = {
                                username,
                                role
                            };
                            let newFieldsCopy = { ...this.state.newFields };
                            newFieldsCopy.isNew = false;
                            this.setState({
                                users: usersCopy,
                                newFields: newFieldsCopy
                            });
                        }
                    })
                    .catch((error) => {
                        throw new Error(`ERROR in USER_ADMINISTRATION/createUser: ${error}`);
                    })
                
            } else {
                let uri = `/api/edituser?username=${encodeURIComponent(user.username)}`;
                if (this.state.newFields.password !== '') uri += `&password=${encodeURIComponent(this.state.newFields.password)}`;
                if (this.state.newFields.role !== user.role) uri += `&role=${encodeURIComponent(this.state.newFields.role)}`;
                if(this.state.newFields.resetMfa) uri +=  `&resetmfa=${encodeURIComponent
                (this.state.newFields.resetMfa)}`
                fetch(uri, {
                    method: 'POST',
                    credentials: 'include',
                })
                    .then((response) => response.json())
                    .then((response) => {
                        this.props.dispatch(setPopupAlert(response.status, response.message, true))
                    })
                    .catch((error) => {
                        throw new Error(`ERROR in USER_ADMINISTRATION/editUser: ${error}`);
                    });
            }
        } else {
            let usersCopy = this.state.users.filter((user) => !user.isNew);
            this.setState({
                newFields: {
                    password: '',
                    role: user.role
                },
                users: usersCopy,
                selectedIndex: null
            })
        }
    }

    handleResetMFA(event) {
        this.setState({
            newFields: {
                username: this.state.newFields.username,
                password: this.state.newFields.password,
                role: this.state.newFields.role,
                resetMfa: event.target.checked
            }
        })
    }

     componentDidMount(){
        getDataForURI('site-admin/read/summary')
        .then((response) => response.json())
        .then((response) => {
            this.setState({siteUsesMfa: response.message.password.multi_factor_authentication})
        })
        .catch((error) => {
            console.log(`ERROR in SITE_ADMINISTRATION/componentDidMount: ${error}`);
        });
     }
    render() {
        const { classes } = this.props;
        const { isLoading, users, selectedIndex, readyDelete, newFields, selectedRole, showAlert, popupMeta } = this.state;

        const roles = userRole === 'developer' ? ['none', 'developer', ...ROLES] : ['none', ...ROLES];

        return (
            <div>
                <Box id="use-this-one" style={{ display: 'flex', flexDirection: 'row', justifyContent: 'space-between' }}>
                    <FormControl variant="outlined">
                        <InputLabel>Roles</InputLabel>
                        <Select
                            id="user-roles"
                            label="Roles"
                            value={selectedRole}
                            onChange={this.handleSelectRole}
                            style={{ width: '240px' }}
                        >
                            {roles.map((item, index) => (
                                <MenuItem id={item} key={index} value={item}>{capitalizeFirst(item)}</MenuItem>
                            ))}
                        </Select>
                        <FormHelperText>Select a user role to view</FormHelperText>
                    </FormControl>
                    <SaveCancel
                        handleSave={this.handleSave}
                        disabled={ selectedIndex === null }
                    />
                </Box>
                <div style={{ display: 'flex', height: '72vh'  }}>
                    <div style={{ width: '24vw', minWidth: '350px', height: '100%', marginRight: '10px' }}>
                        <Paper style={{ height: '100%', width: '100%', marginBottom: '10px', overflow: 'auto', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                            <List subheader={<li />} style={{ flexGrow: 1, width: '100%' }}>
                                <ListSubheader className={classes.subheader}>
                                    Users
                                </ListSubheader>
                                {isLoading && <Loading size={50} style={{ height: '100%' }} />}
                                {(!isLoading && users) && users.map((user, index) => (
                                    <ListItem
                                        className="roles"
                                        id={(user.username.split(' ').join('')).toLowerCase()}
                                        key={index}
                                        onClick={() => this.handleSelectUser(user, index)}
                                        selected={selectedIndex === index}
                                        button
                                    >
                                        <ListItemText>{user.username}</ListItemText>
                                    </ListItem>
                                ))}
                            </List>
                            <Button
                                id="add-user"
                                variant={'contained'}
                                color="primary"
                                style={{ minHeight: '36px', width: '90%', marginBottom: '20px' }}
                                onClick={() => this.addUser()}
                                disabled={selectedRole === 'none'}
                            >
                                <Add style={{ marginRight: '5px' }} />
                                Add User
                            </Button>
                        </Paper>
                    </div>
                    <div style={{ display: 'flex', width: '60vw', flexGrow: 1 }}>
                        <Paper style={{  width: '100%', height: '100%' }}>
                            <List subheader={<li />}>
                                <ListSubheader className={classes.subheader}>
                                    Info
                                </ListSubheader>
                                {selectedIndex !== null &&
                                    <React.Fragment>
                                        <ListItem
                                            style={{ display: 'flex', justifyContent: 'space-between', width: '60%' }}
                                        >
                                            <ListItemText>Username</ListItemText>
                                            {newFields.isNew &&
                                                <TextInput
                                                    value={this.state.newFields.username}
                                                    handleChange={(event) => this.handleChangeUser(event, 'username')}
                                                />
                                            }
                                            {!newFields.isNew &&
                                                <ListItemText style={{ 'textAlign': 'right' }}>{users[selectedIndex].username}</ListItemText>
                                            }
                                        </ListItem>
                                        <ListItem
                                            style={{ display: 'flex', justifyContent: 'space-between', width: '60%' }}
                                        >
                                            <ListItemText>New Password</ListItemText>
                                            <TextInput
                                                value={newFields.password}
                                                handleChange={(event) => this.handleChangeUser(event, 'password')}
                                                password
                                            />
                                        </ListItem>
                                        <ListItem
                                            style={{ display: 'flex', justifyContent: 'space-between', width: '60%' }}
                                        >
                                            <ListItemText>Role</ListItemText>
                                            <FormControl variant="outlined">
                                                <Select
                                                    id="new-role"
                                                    value={newFields.role}
                                                    onChange={(event) => this.handleChangeUser(event, 'role')}
                                                    style={{ 'minWidth': 100 }}
                                                >
                                                    {ROLES.map((role, index) => (
                                                        <MenuItem value={role}>
                                                            {capitalizeFirst(role)}
                                                        </MenuItem>
                                                    ))}
                                                </Select>
                                            </FormControl>
                                        </ListItem>
                                       {(this.state.siteUsesMfa && !newFields.isNew) ? <ListItem
                                            style={{ display: 'flex', justifyContent: 'space-between', width: '60%' }}
                                        >
                                            <ListItemText>Enable Secret Recovery</ListItemText>
                                            <Switch onChange={this.handleResetMFA.bind(this)} color="primary"/>
                                        </ListItem> : <></>}
                                        <ListItem
                                            style={{ display: 'flex', justifyContent: 'space-between', width: '60%' }}
                                        >
                                            <Button
                                                variant={readyDelete ? 'contained' : 'outlined'}
                                                color="secondary"
                                                style={{ minHeight: '42px', minWidth: '120px', marginRight: '10px' }}
                                                onClick={() => this.setState({ readyDelete: !readyDelete })}
                                            >
                                                Delete
                                            </Button>
                                            <Confirm
                                                disabled={!readyDelete}
                                                handleConfirm={this.handleDelete}
                                            />
                                        </ListItem>
                                    </React.Fragment>
                                }
                            </List>
                        </Paper>
                    </div>
                </div>
            </div>
        );
    }
}

export default withStyles(connect()(UserAdministration), STYLES_USER_ADMIN);