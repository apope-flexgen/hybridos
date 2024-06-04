/* eslint-disable max-lines */
import {
  Box, Divider, Modal, MuiButton, ThemeType, Typography,
} from '@flexgen/storybook';
import { useContext, useEffect, useState } from 'react';

import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import DeleteOrganizationsBanner from 'src/pages/ActivityLog/AlertManagement/AlertForm/OrganizationsModal/DeleteOrganizationsBanner/DeleteOrganizationsBanner';
import OrganizationRow from 'src/pages/ActivityLog/AlertManagement/AlertForm/OrganizationsModal/OrganizationRow/OrganizationRow';
import { alertManagementHelperText } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  formRowSx,
  orgRowSx,
  saveDisabledErrorBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';

import {
  alertModalSx,
  cancelButtonSx,
  modalButtonsSx,
  saveButtonSx,
} from 'src/pages/ActivityLog/activityLog.styles';
import { Organization } from 'src/pages/ActivityLog/activityLog.types';
import { useTheme } from 'styled-components';

export interface EditOrganizationsModalProps {
  open: boolean;
  organizations: Organization[];
  onClose: () => void;
  setOrganizations: React.Dispatch<React.SetStateAction<Organization[]>>;
  handleCancel: () => void;
}

const EditOrganizationsModal: React.FC<EditOrganizationsModalProps> = ({
  open,
  organizations,
  onClose,
  setOrganizations,
  handleCancel,
}: EditOrganizationsModalProps) => {
  const [emptyError, setEmptyError] = useState<boolean>(false);
  const [duplicateNamesError, setDuplicateNamesError] = useState<boolean>(false);

  const [updatedOrganizations, setUpdatedOrganizations] = useState<Organization[]>(organizations);
  const theme = useTheme() as ThemeType;

  const generateOrganizationRows = () => updatedOrganizations
    .sort((a, b) => (a.id || '').toString().localeCompare(b.id || '', undefined, { numeric: true }))
    .map((organization) => (
      <OrganizationRow
        organization={organization}
        setUpdatedOrganizations={setUpdatedOrganizations}
        onClose={onClose}
        handleCancel={handleCancel}
      />
    ));

  const [organizationRows, setOrganizationRows] = useState<JSX.Element[]>(
    generateOrganizationRows(),
  );

  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const addNewRow = () => {
    setUpdatedOrganizations((prevState) => [
      ...prevState,
      { name: '', id: `temp${updatedOrganizations.length + 1}` },
    ]);
  };

  const handleSubmit = () => {
    if (emptyError) {
      return;
    }

    const organizationsWithoutTempIDs = updatedOrganizations.map((organization) => {
      if (organization.id && organization.id.includes('temp')) return { name: organization.name };
      return organization;
    });

    axiosInstance
      .post('/alerts/organizations', { organizations: organizationsWithoutTempIDs })
      .then((res) => {
        if (res.data.success) {
          setOrganizations(
            organizationsWithoutTempIDs.sort((a, b) => a.name.localeCompare(b.name)),
          );
          notifCtx?.notif('success', 'Organizations successfully updated');
          onClose();
        } else {
          notifCtx?.notif('error', `Error updating organizations: ${res.data.body.message}`);
          onClose();
        }
      });
  };

  useEffect(() => {
    const uniqueNames = new Set(
      updatedOrganizations.map((organization) => organization.name.toLowerCase()),
    );
    if (updatedOrganizations.length !== [...uniqueNames].length) {
      setDuplicateNamesError(true);
    } else setDuplicateNamesError(false);
    if (
      updatedOrganizations.length === 0
      || updatedOrganizations.some((organization) => !organization.name)
    ) setEmptyError(true);
    else setEmptyError(false);
    setOrganizationRows(generateOrganizationRows());
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [updatedOrganizations]);

  useEffect(() => {
    setUpdatedOrganizations(organizations);
  }, [organizations, open]);

  return (
    <Modal title="Edit Organizations" open={open} onClose={onClose}>
      <Box sx={alertModalSx}>
        <DeleteOrganizationsBanner />
        <Typography text={alertManagementHelperText.editOrganizations} variant="bodyS" />
        <Divider orientation="horizontal" variant="fullWidth" />
        <Box sx={orgRowSx(theme)}>{organizationRows}</Box>
        <Box sx={formRowSx}>
          <MuiButton label="Add Organization" onClick={addNewRow} startIcon="Add" variant="text" />
        </Box>
        <Divider orientation="horizontal" variant="fullWidth" />
        <Box sx={modalButtonsSx}>
          <MuiButton label="Cancel" variant="outlined" onClick={onClose} sx={cancelButtonSx} />
          <MuiButton
            label="Submit"
            onClick={handleSubmit}
            sx={saveButtonSx}
            disabled={emptyError || duplicateNamesError}
          />
        </Box>
        <Box sx={saveDisabledErrorBoxSx}>
          {emptyError && (
            <Typography
              text={alertManagementHelperText.orgEmptyError}
              variant="bodySBold"
              color="error"
            />
          )}
          {duplicateNamesError && (
            <Typography
              text={alertManagementHelperText.orgDuplicateError}
              variant="bodySBold"
              color="error"
            />
          )}
        </Box>
      </Box>
    </Modal>
  );
};

export default EditOrganizationsModal;
