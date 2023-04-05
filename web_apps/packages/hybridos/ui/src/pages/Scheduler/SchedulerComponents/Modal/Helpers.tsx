export const labels = {
  unsavedChanges: {
    title: 'Unsaved Changes',
    description: 'Your changes will be lost if you dont save them.',
    secondaryLabel: 'Discard & Exit',
    primaryLabel: 'Save & Exit',
  },
  deleteConfirmation: {
    title: 'Are you sure?',
    description:
            'All events using this Mode will also be deleted. This action cannot be undone.',
    secondaryLabel: 'Cancel',
    primaryLabel: 'Delete Mode',
  },
  navigateWithoutSaving: {
    title: 'Unsaved Changes',
    description: 'You have unsaved changes in this tab. They will be lost if you do not save them.',
    secondaryLabel: 'Discard & Continue',
    primaryLabel: '',
  },
};

export type ModalStateType = {
  type: SchedulerModalTypes,
  onClose: () => void,
  primaryActions: any,
  secondaryActions: any,
};

export type SchedulerModalTypes = 'unsavedChanges' | 'deleteConfirmation' | 'navigateWithoutSaving';
