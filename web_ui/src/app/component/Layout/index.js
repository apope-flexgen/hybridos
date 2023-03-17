/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React, { useState } from 'react';
// TODO: DO NOT DELETE: the "import io" line is apparently required for the line:
// `socket_heart1000.io.opts.transports = ['polling', 'websocket'];`
// eslint-disable-next-line no-unused-vars
import io from 'socket.io-client';
import PropTypes from 'prop-types';
import classNames from 'classnames';

import { Scrollbars } from 'react-custom-scrollbars-2';
import { withStyles } from 'tss-react/mui';
import Button from '@mui/material/Button';
import CssBaseline from '@mui/material/CssBaseline';
import Drawer from '@mui/material/Drawer';
import AppBar from '@mui/material/AppBar';
import Toolbar from '@mui/material/Toolbar';
import List from '@mui/material/List';
import Typography from '@mui/material/Typography';
import Divider from '@mui/material/Divider';
import IconButton from '@mui/material/IconButton';
import MenuIcon from '@mui/icons-material/Menu';
import DashboardIcon from '@mui/icons-material/Dashboard';
import ChevronLeftIcon from '@mui/icons-material/ChevronLeft';
import { Link, useLocation, useNavigate, useParams } from 'react-router-dom';
import MenuItem from '@mui/material/MenuItem';
import ListItemIcon from '@mui/material/ListItemIcon';
import ListItemText from '@mui/material/ListItemText';
import ListSubheader from '@mui/material/ListSubheader';
import Home from '@mui/icons-material/Home';
import SettingsApplications from '@mui/icons-material/SettingsApplications';
import Power from '@mui/icons-material/Power';
import SettingsBackupRestore from '@mui/icons-material/SettingsBackupRestore';
import WbSunny from '@mui/icons-material/WbSunny';
import FormatListBulleted from '@mui/icons-material/FormatListBulleted';
import Place from '@mui/icons-material/Place';
import BatteryChargingFull from '@mui/icons-material/BatteryChargingFull';
import SmsIcon from '@mui/icons-material/Sms';
import DeveloperBoardIcon from '@mui/icons-material/DeveloperBoard';
import BarChartIcon from '@mui/icons-material/BarChart';
import SaveAltIcon from '@mui/icons-material/SaveAlt';
import InfoIcon from '@mui/icons-material/Info';
import KitchenIcon from '@mui/icons-material/Kitchen';
import EditIcon from '@mui/icons-material/Edit';
import AppsIcon from '@mui/icons-material/Apps';
import ScheduleIcon from '@mui/icons-material/Schedule';
import GroupAddIcon from '@mui/icons-material/GroupAdd';
import AdminPanelSettingsIcon from '@mui/icons-material/AdminPanelSettings';
import MoreTimeIcon from '@mui/icons-material/MoreTime';
import AppRegistrationIcon from '@mui/icons-material/AppRegistration';
import RadioGroupNoControl from '../RadioGroupNoControl';
import ThroughputDisplay from "../ThroughputDisplay";
import TuneIcon from '@mui/icons-material/Tune';
import Card from "@mui/material/Card";
import MultiUseModal from "../MultiUseModal/index";
import ExitToAppIcon from '@mui/icons-material/ExitToApp';
import { STYLES_LAYOUT } from '../../styles';
import {
  DASHBOARD_PAGE_PATH,
  SITE_PAGE_PATH,
  FEATURES_PAGE_PATH,
  EVENTS_PAGE_PATH,
  ESS_PAGE_PATH,
  GENERATOR_PAGE_PATH,
  SOLAR_PAGE_PATH,
  FEEDERS_PAGE_PATH,
  CONTROL_CABINET_PAGE_PATH,
  USER_ADMINISTRATION_PAGE_PATH,
  SITE_ADMINISTRATION_PAGE_PATH,
  COMPONENTS_PAGE_PATH,
  FIMS_PAGE_PATH,
  CHARTS_PAGE_PATH,
  DBI_DOWNLOAD_PAGE_PATH,
  SYSTEM_INFORMATION_PAGE_PATH,
  SCHEDULER_PAGE_PATH,
  FLEXGEN_LOGO,
  FLEXGEN_LOGO2,
  BMS_PAGE_PATH,
  PCS_PAGE_PATH,
  SCHEDULER_CONFIGURATION_PATH,
  WEBUI_CONFIG_PATH,
  VAR_OVERRIDE_PAGE_PATH
} from "../../../AppConfig";
import {
  siteConfiguration,
  socket_heart1000,
  socket_serverCount,
  confirmRoleAccess,
  doLogout,
} from "../../../AppAuth";

import PopupAlert from '../PopupAlert';

let hostIsLocalhost = false;

const TRUNCATE_LEN = 16;

export function consoleLog(message) {
  if (hostIsLocalhost) console.log(message);
}

function shortLabel(isOpen, name) {
  if (!isOpen) {
    return '';
  }
  if (name.length > TRUNCATE_LEN + 3) {
    return name.substring(0, TRUNCATE_LEN) + '...';
  } else {
    return name;
  }
}

function DrawerTab(props) {
  return (
    <MenuItem
      button
      id={"siteButton"}
      component={Link}
      to={props.path}
      selected={props.pathname === props.path}
      style={{ paddingLeft: 24 }}
    >
      <ListItemIcon>
        <AppsIcon />
      </ListItemIcon>
      <ListItemText style={{ maxWidth: 160, overflow: 'hidden' }} primary={shortLabel(props.isOpen, props.name)} />
    </MenuItem>
  );
}

/**
 * Renders side menu layout
 */
class Layout extends React.PureComponent {
  constructor(props) {
    super(props);
    this.state = {
      siteConfiguration: {},
      isLoggedIn: true,
      open: true,
      countUpdateOutput: "",
    };
  }

  /**
   * Socket connection setup
   */
  // eslint-disable-next-line class-methods-use-this
  componentDidMount() {
    hostIsLocalhost = window.location.hostname === "localhost";
    // in a production environment, websocket should be available unless a
    // proxy or firewall is specifically disallowing it. In that case we
    // need to fallback to polling which is more server-intensive. See notes
    // in AppConfig.js for more information related to the following function.
    // IMPORTANT EXPLANATION: the following code causes all of socket.io to
    // reset its transports if the reconnect_attempt block kicks-in. This code
    // tests the connection - and then adjusts it if necessary - in a
    // predictable, reliable way
    setTimeout(() => {
      socket_heart1000.on("/heart1000", () => {
        this.setState({ siteConfiguration });
        // sets it from AppAuth once heartbeat has begun
        // this is just a test handshake when web_ui loads. If we get a
        // response then the socket.io transport is working. This test is
        // done so we turn off the socket.
        socket_heart1000.off("/heart1000");
      });
      socket_heart1000.on("reconnect_attempt", () => {
        // if the handshake above does not get a response then we will
        // reset the transports to use the slower, more server-intensive
        // polling. Then we'll turn off the socket and click the Dashboard
        // on the sidebar menu to "reload" the page in a way that doesn't
        // blow up the reconnection as a real reload would.
        // eslint-disable-next-line no-param-reassign
        socket_heart1000.io.opts.transports = ["polling", "websocket"];
        socket_heart1000.off("/heart1000");
        consoleLog(
          "NOTE: websocket not available, socket transports falling back to polling."
        );
      });
    }, 2000);
    document.getElementById("dashboardButton").click(); // this gets us to the default page.
  }

  /**
   * Socket unmount
   */
  // eslint-disable-next-line class-methods-use-this
  componentWillUnmount() {
    socket_heart1000.off("/heart1000");
    socket_serverCount.off("/serverCount");
  }

  /**
   * Sets drawer open to false in state
   */
  handleDrawerOpen = () => {
    this.setState({ open: true });
  };

  /**
   * Sets drawer open to false in state
   */
  handleDrawerClose = () => {
    this.setState({ open: false });
  };


  render() {
    const {
      classes,
      location: { pathname },
      children
    } = this.props;
    // ^ this keeps the radio buttons updated
    // API calls cannot happen without authorization, but a socket.io socket could
    // be left open without a logged in user under some circumstances, so we'll
    // make sure it's disconnected here.
    return (
      <React.Fragment>
        <CssBaseline />
        <div className={classes.root}>
          <AppBar className={classes.appBar}>
            <Toolbar
              disableGutters={!this.state.open}
              className={classes.toolbar}
            >
              <div
                className={classes.toolbarIcon}
                style={{
                  backgroundColor: "white",
                  height: 64,
                  width: 255,
                  marginLeft: this.state.open ? -35 : -290,
                  marginRight: 14,
                  transitionProperty: "all",
                }}
              >
                <img
                  data-cy="flexgen_logo2"
                  src={FLEXGEN_LOGO2}
                  className={classes.logo}
                  alt="FlexGen"
                  style={{ marginBottom: 5, marginRight: 15 }}
                />

                <IconButton
                  data-cy="chevron_left_icon"
                  onClick={this.handleDrawerClose}
                  style={{ marginBottom: 5 }}
                >
                  <ChevronLeftIcon />
                </IconButton>
              </div>
              <Divider
                orientation="vertical"
                flexItem
                style={{ marginLeft: -15, marginRight: 24 }}
              />
              <IconButton
                data-cy="hamburger_icon"
                color="inherit"
                aria-label="Open drawer"
                onClick={this.handleDrawerOpen}
                className={classNames(
                  classes.menuButton,
                  this.state.open && classes.menuButtonHidden
                )}
                style={{ left: 12 }}
              >
                <MenuIcon />
              </IconButton>
              <Typography
                variant="subtitle1"
                color="inherit"
                noWrap
                className={classes.title}
                id="page-title"
              >
                {pathname === WEBUI_CONFIG_PATH && "UI Configuration"}
                {pathname === DASHBOARD_PAGE_PATH && "Dashboard"}
                {pathname === SITE_PAGE_PATH && "Site"}
                {pathname === FEATURES_PAGE_PATH && "Features"}
                {pathname === EVENTS_PAGE_PATH && "Events"}
                {pathname === ESS_PAGE_PATH && "Assets / Storage"}
                {pathname === GENERATOR_PAGE_PATH && "Assets / Generators"}
                {pathname === SOLAR_PAGE_PATH && "Assets / Solar"}
                {pathname === FEEDERS_PAGE_PATH && "Assets / Feeders"}
                {pathname === VAR_OVERRIDE_PAGE_PATH && "Variable Override"}
                {pathname === CONTROL_CABINET_PAGE_PATH && "Control Cabinet"}
                {pathname === USER_ADMINISTRATION_PAGE_PATH &&
                  "Administration / User Administration"}
                {pathname === SITE_ADMINISTRATION_PAGE_PATH &&
                  "Administration / Site Administration"}
                {pathname === COMPONENTS_PAGE_PATH && "Inspector / Components"}
                {pathname === FIMS_PAGE_PATH && "Inspector / FIMS"}
                {pathname === DBI_DOWNLOAD_PAGE_PATH &&
                  "Inspector / DBI Upload/Download"}
                {pathname === SYSTEM_INFORMATION_PAGE_PATH &&
                  "Inspector / System Information"}
                {pathname === SCHEDULER_PAGE_PATH && "Scheduler"}
                {pathname === SCHEDULER_CONFIGURATION_PATH &&
                  "Scheduler Configuration"}
              </Typography>
              {this.state.showPasswordInput ? (
                <input
                  data-cy="password_input"
                  id="password"
                  className="password-input"
                  type="password"
                  onKeyUp={this.handleInspectorOpen}
                  style={{ opacity: 0.05, marginRight: "10px" }}
                ></input>
              ) : null}
             
             <Typography
                variant="h6"
                color="inherit"
                noWrap
                className={classes.siteName}
              >
                {siteConfiguration.siteName}
              </Typography>
              
            </Toolbar>
            <MultiUseModal />
          </AppBar>
          <Drawer
            data-cy="layout_drawer"
            variant="permanent"
            open={this.state.open}
            classes={{
              paper: classNames(
                classes.drawerPaper,
                !this.state.open && classes.drawerPaperClose
              ),
            }}
          >
            <Divider />
            <Scrollbars>
              <List>
                <MenuItem
                  button
                  id={"dashboardButton"}
                  component={Link}
                  to={DASHBOARD_PAGE_PATH}
                  selected={pathname === DASHBOARD_PAGE_PATH}
                  style={{ paddingLeft: 24 }}
                >
                  <ListItemIcon>
                    <Home />
                  </ListItemIcon>
                  <ListItemText
                    primary={shortLabel(this.state.open, "Dashboard")}
                  />
                </MenuItem>
                {this.state.siteConfiguration.includeSite && (
                  <MenuItem
                    button
                    id={"siteButton"}
                    component={Link}
                    to={SITE_PAGE_PATH}
                    selected={pathname === SITE_PAGE_PATH}
                    style={{ paddingLeft: 24 }}
                  >
                    <ListItemIcon>
                      <Place />
                    </ListItemIcon>
                    <ListItemText primary={shortLabel(this.state.open, "Site")} />
                  </MenuItem>
                )}
                {this.state.siteConfiguration.includeFeatures && (
                  <MenuItem
                    button
                    component={Link}
                    to={FEATURES_PAGE_PATH}
                    selected={pathname === FEATURES_PAGE_PATH}
                    style={{ paddingLeft: 24 }}
                  >
                    <ListItemIcon>
                      <SettingsApplications />
                    </ListItemIcon>
                    <ListItemText
                      primary={shortLabel(this.state.open, "Features")}
                    />
                  </MenuItem>
                )}
                {this.state.siteConfiguration.includeEvents && (
                  <MenuItem
                    button
                    component={Link}
                    to={EVENTS_PAGE_PATH}
                    selected={pathname === EVENTS_PAGE_PATH}
                    style={{ paddingLeft: 24 }}
                  >
                    <ListItemIcon>
                      <FormatListBulleted />
                    </ListItemIcon>
                    <ListItemText
                      primary={shortLabel(this.state.open, "Events")}
                    />
                  </MenuItem>
                )}
                {this.state.siteConfiguration.includeScheduler &&
                  confirmRoleAccess("/scheduler/events") && (
                    <MenuItem
                      button
                      component={Link}
                      to={SCHEDULER_PAGE_PATH}
                      selected={pathname === SCHEDULER_PAGE_PATH}
                      style={{ paddingLeft: 24 }}
                    >
                      <ListItemIcon>
                        <ScheduleIcon />
                      </ListItemIcon>
                      <ListItemText
                        primary={shortLabel(this.state.open, "Scheduler")}
                      />
                    </MenuItem>
                  )}
              </List>
              <Divider />
              <List>
                {(this.state.open && !this.state.siteConfiguration.fleetManager) ? (
                    <ListSubheader disableSticky={true} className={classes.subheader}>Assets</ListSubheader>
                ) : (this.state.open && this.state.siteConfiguration.fleetManager) ? ( 
                    <ListSubheader disableSticky={true} className={classes.subheader}>Sites</ListSubheader>
                ) : (
                  <></>
                )}
                {this.state.siteConfiguration.fleetName == "ERCOT" && 
                (
                  <MenuItem
                    button
                    component={Link}
                    to={VAR_OVERRIDE_PAGE_PATH}
                    selected={pathname === VAR_OVERRIDE_PAGE_PATH}
                    style={{ paddingLeft: 24 }}
                  >
                    <ListItemIcon>
                      <EditIcon />
                    </ListItemIcon>
                    <ListItemText
                        primary={shortLabel(this.state.open, 
                          "Variable Override"
                        )}
                      />
                  </MenuItem>
                )}
                {this.props.layout &&
                  this.props.layout.map((tab) => (
                    <DrawerTab
                      path={`/${tab.info.key}`}
                      pathname={pathname}
                      name={tab.info.name}
                      isOpen={this.state.open}
                    />
                  ))}
                {this.state.siteConfiguration.includeStorage && (
                  <MenuItem
                    button
                    component={Link}
                    to={ESS_PAGE_PATH}
                    selected={pathname === ESS_PAGE_PATH}
                    style={{ paddingLeft: 24 }}
                  >
                    <ListItemIcon>
                      <BatteryChargingFull />
                    </ListItemIcon>
                    <ListItemText
                      primary={shortLabel(this.state.open, "Storage")}
                    />
                  </MenuItem>
                )}
                {this.state.siteConfiguration.includeGenerator && (
                  <MenuItem
                    button
                    component={Link}
                    to={GENERATOR_PAGE_PATH}
                    selected={pathname === GENERATOR_PAGE_PATH}
                    style={{ paddingLeft: 24 }}
                  >
                    <ListItemIcon>
                      <SettingsBackupRestore />
                    </ListItemIcon>
                    <ListItemText
                      primary={shortLabel(this.state.open, "Generator")}
                    />
                  </MenuItem>
                )}
                {this.state.siteConfiguration.includeSolar && (
                  <MenuItem
                    button
                    component={Link}
                    to={SOLAR_PAGE_PATH}
                    selected={pathname === SOLAR_PAGE_PATH}
                    style={{ paddingLeft: 24 }}
                  >
                    <ListItemIcon>
                      <WbSunny />
                    </ListItemIcon>
                    <ListItemText primary={shortLabel(this.state.open, "Solar")} />
                  </MenuItem>
                )}
                {this.state.siteConfiguration.includeGrid && (
                  <MenuItem
                    button
                    component={Link}
                    to={FEEDERS_PAGE_PATH}
                    selected={pathname === FEEDERS_PAGE_PATH}
                    style={{ paddingLeft: 24 }}
                  >
                    <ListItemIcon>
                      <Power />
                    </ListItemIcon>
                    <ListItemText
                      primary={shortLabel(this.state.open, "Feeders")}
                    />
                  </MenuItem>
                )}
                {this.state.siteConfiguration.includeControlCabinet &&
                  confirmRoleAccess("/control_cabinet/control_cabinet") && (
                    <MenuItem
                      button
                      component={Link}
                      to={CONTROL_CABINET_PAGE_PATH}
                      selected={pathname === CONTROL_CABINET_PAGE_PATH}
                      style={{ paddingLeft: 24 }}
                    >
                      <ListItemIcon>
                        <KitchenIcon />
                      </ListItemIcon>
                      <ListItemText
                        primary={shortLabel(this.state.open, "Control Cabinet")}
                      />
                    </MenuItem>
                  )}
              </List>
              {confirmRoleAccess("/administration/useradministration") && (
                <>
                  <Divider />
                  <List id="administration">
                    {this.state.open ? (
                      <ListSubheader disableSticky={true} className={classes.subheader}>Administration</ListSubheader>
                    ) : (
                      <></>
                    )}
                    <MenuItem
                      id = 'user_admin'
                      button
                      component={Link}
                      to={USER_ADMINISTRATION_PAGE_PATH}
                      selected={pathname === USER_ADMINISTRATION_PAGE_PATH}
                      style={{ paddingLeft: 24 }}
                    >
                      <ListItemIcon>
                        <GroupAddIcon />
                      </ListItemIcon>
                      <ListItemText
                        primary={shortLabel(this.state.open, 
                          "User Administration"
                        )}
                      />
                    </MenuItem>
                    <MenuItem
                      button
                      component={Link}
                      to={SITE_ADMINISTRATION_PAGE_PATH}
                      selected={pathname === SITE_ADMINISTRATION_PAGE_PATH}
                      style={{ paddingLeft: 24 }}
                    >
                      <ListItemIcon>
                        <AdminPanelSettingsIcon />
                      </ListItemIcon>
                      <ListItemText
                        primary={shortLabel(this.state.open, 
                          "Site Administration"
                        )}
                      />
                    </MenuItem>
                    {this.state.siteConfiguration.includeScheduler &&
                      confirmRoleAccess("/scheduler/modes") && (
                        <MenuItem
                          button
                          component={Link}
                          to={SCHEDULER_CONFIGURATION_PATH}
                          selected={pathname === SCHEDULER_CONFIGURATION_PATH}
                          style={{ paddingLeft: 24 }}
                        >
                          <ListItemIcon>
                            <MoreTimeIcon />
                          </ListItemIcon>
                          <ListItemText
                            primary={shortLabel(this.state.open, 
                              "Scheduler Configuration"
                            )}
                          />
                        </MenuItem>
                      )}
                    <MenuItem
                      button
                      id = 'ui-config'
                      component={Link}
                      to={WEBUI_CONFIG_PATH}
                      selected={pathname === WEBUI_CONFIG_PATH}
                      style={{ paddingLeft: 24 }}
                    >
                      <ListItemIcon>
                        <AppRegistrationIcon />
                      </ListItemIcon>
                      <ListItemText
                        primary={shortLabel(this.state.open, "UI Configuration")}
                      />
                    </MenuItem>
                  </List>
                </>
              )}
              {confirmRoleAccess("/inspector") && (
                <>
                  <Divider />
                  <List id="inspector">
                    {this.state.open ? (
                      <ListSubheader  disableSticky={true} className={classes.subheader}>Inspector</ListSubheader>
                    ) : (
                      <></>
                    )}
                    {this.state.siteConfiguration.inspectorComponentsName &&
                    confirmRoleAccess("/inspector/components") ? (
                      <MenuItem
                        button
                        component={Link}
                        to={COMPONENTS_PAGE_PATH}
                        selected={pathname === COMPONENTS_PAGE_PATH}
                        style={{ paddingLeft: 24 }}
                      >
                        <ListItemIcon>
                          <DeveloperBoardIcon />
                        </ListItemIcon>
                        <ListItemText
                          primary={shortLabel(this.state.open, "Components")}
                        />
                      </MenuItem>
                    ) : (
                      ""
                    )}
                    {confirmRoleAccess("/inspector/fims") ? (
                      <MenuItem
                        button
                        id={"inspectorFimsButton"}
                        component={Link}
                        to={FIMS_PAGE_PATH}
                        selected={pathname === FIMS_PAGE_PATH}
                        style={{ paddingLeft: 24 }}
                      >
                        <ListItemIcon>
                          <SmsIcon />
                        </ListItemIcon>
                        <ListItemText
                          primary={shortLabel(this.state.open, "FIMS")}
                        />
                      </MenuItem>
                    ) : (
                      ""
                    )}
                    {confirmRoleAccess("/inspector/dbiDownload") ? (
                      <MenuItem
                        button
                        id={"inspectorDBIDownloadButton"}
                        component={Link}
                        to={DBI_DOWNLOAD_PAGE_PATH}
                        selected={pathname === DBI_DOWNLOAD_PAGE_PATH}
                        style={{ paddingLeft: 24 }}
                      >
                        <ListItemIcon>
                          <SaveAltIcon />
                        </ListItemIcon>
                        <ListItemText
                          primary={shortLabel(this.state.open, 
                            "DBI Upload/Download"
                          )}
                        />
                      </MenuItem>
                    ) : (
                      ""
                    )}
                    <Card
                      style={{ marginTop: 10, marginLeft: 30, width: "12em" }}
                    >
                      {this.state.open ? (
                        <ThroughputDisplay
                          pathname={pathname}
                          classes={classes}
                        />
                      ) : (
                        <></>
                      )}
                    </Card>
                  </List>
                </>
              )}
              <Divider />
              <div style={{ textAlign: "center" }}>
                <Button
                  id="logout-button"
                  style={{ margin: 10, minWidth: 40, paddingLeft: 13, paddingRight: 13 }}
                  variant="contained"
                  color="primary" // this is the bg color of the button
                  onClick={doLogout}
                >
                  {this.state.open ? (
                    "Logout"
                  ) : (
                    <ExitToAppIcon fontSize="small" />
                  )}
                </Button>
              </div>
            </Scrollbars>
          </Drawer>
          <main
            className={classNames(
              !this.state.open && classes.content,
              this.state.open && classes.contentShift
            )}
          >
            <div className={classes.appBarSpacer} />
            {children}
          </main>
          <PopupAlert />   
          </div>
      </React.Fragment>
    );
  }
}
Layout.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Bandaid fix to replicate old withRouter functionality
// https://github.com/remix-run/react-router/issues/7361#issuecomment-789920301
export const withRouter = ComponentWithRouter => props => {
    let location = useLocation();
    let match = { params: useParams() };
    // https://reactrouter.com/docs/en/v6/upgrading/v5#use-usenavigate-instead-of-usehistory
    let navigate = useNavigate();
    let history = {
        back: () => navigate(-1),
        goBack: () => navigate(-1),
        location,
        push: (url, state) => navigate(url, { state }),
        replace: (url, state) => navigate(url, { replace: true, state }),
    };
    return <ComponentWithRouter
        location={location}
        navigate={navigate}
        match={match}
        history={history}
        {...props}
    />
}

// Not sure if withTheme is needed
// export default withRouter(
//         withStyles(Layout, 
//             (STYLES_LAYOUT, { withTheme: true })
//         )
//     );
export default withRouter(withStyles(Layout, STYLES_LAYOUT));
