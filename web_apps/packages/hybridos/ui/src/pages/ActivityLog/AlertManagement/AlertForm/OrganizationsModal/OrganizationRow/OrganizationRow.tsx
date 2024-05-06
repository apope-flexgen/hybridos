import { CheckXConfirm, IconButton, TextField } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { useContext, useState } from 'react';

import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { comparator1RowSx } from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Organization } from 'src/pages/ActivityLog/activityLog.types';

export interface OrganizationRowProps {
  setUpdatedOrganizations: React.Dispatch<React.SetStateAction<Organization[]>>;
  organization: Organization;
  onClose: () => void;
  handleCancel: () => void;
}

const OrganizationRow = ({
  setUpdatedOrganizations,
  organization,
  onClose,

  handleCancel,
}: OrganizationRowProps) => {
  const [showCheckXConfirm, setShowCheckXConfirm] = useState<boolean>(false);
  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const handleValueFieldChange = (newValue: string) => {
    setUpdatedOrganizations((prevState) => {
      const organizationsWithoutCurrent = prevState.filter(
        (existingOrg) => organization.id !== existingOrg.id,
      );

      const updatedOrg: Organization = { id: organization.id, name: newValue };

      return [...organizationsWithoutCurrent, updatedOrg];
    });
  };

  const handleDelete = () => {
    if (organization.id && organization.id.includes('temp')) {
      setUpdatedOrganizations((prevState) => {
        const organizationsWithoutCurrent = prevState.filter(
          (existingOrg) => organization.id !== existingOrg.id,
        );
        return organizationsWithoutCurrent;
      });
    } else {
      axiosInstance.delete(`/alerts/organizations/${organization.id}`).then((res) => {
        setShowCheckXConfirm(false);
        if (res.data.success) {
          handleCancel();
          notifCtx?.notif('success', 'Organization successfully deleted');
          onClose();
        } else {
          notifCtx?.notif('error', `Error deleting organization: ${res.data.body.message}`);
        }
      });
    }
  };

  return (
    <Box sx={{ ...comparator1RowSx, alignItems: 'flex-start' }}>
      <TextField
        fullWidth
        value={organization.name}
        onChange={(e) => handleValueFieldChange(e.target.value)}
        label=""
      />
      <Box sx={{ display: 'flex' }}>
        <IconButton
          disabled={showCheckXConfirm}
          icon="TrashOutline"
          onClick={() => setShowCheckXConfirm(true)}
          color="error"
        />
        {showCheckXConfirm && (
          <CheckXConfirm onCheck={handleDelete} onX={() => setShowCheckXConfirm(false)} />
        )}
      </Box>
    </Box>
  );
};

export default OrganizationRow;
