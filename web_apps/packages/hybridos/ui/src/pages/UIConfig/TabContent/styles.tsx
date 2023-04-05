import styled from 'styled-components';

export const Container = styled.div`
  height: 100%;
`;

export const ListContainer = styled.div`
  margin-top: 14px;
  display: flex;
  flex-direction: column;
  row-gap: 10px;
`;

export const Item = styled.div`
  display: flex;
  align-items: center;
`;

export const Toolbar = styled.div`
  display: flex;
  justify-content: space-between;
  margin-bottom: 24px;
`;

export const ButtonsContainer = styled.div`
  display: flex;
  column-gap: 24px;
`;

export const AddItemButtonSX = {
  marginTop: '10px',
};

export const TabContainer = styled.div`
  display: flex;
  width: 100%;
  height: calc(100% - 64px);

  .setting {
    margin-right: 80px;
  }
`;

export const MainBoxSX = {
  width: '321px',
  background: 'rgba(0, 104, 119, 0.04)',
  padding: '16px',
  display: 'flex',
  flexDirection: 'column',
  overflow: 'auto',
};

export const BoxSX = {
  width: '100%',
  padding: '24px',
  overflow: 'auto',
};

export const MuiButtonSX = {
  width: '70%',
  marginRight: '24px',
};
