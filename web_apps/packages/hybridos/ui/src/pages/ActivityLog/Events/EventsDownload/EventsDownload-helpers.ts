import jsPDF from 'jspdf';
import autoTable from 'jspdf-autotable';
import * as XLSX from 'xlsx';

const baseFilename = 'events';

export const downloadTxt = (data: Event[], timeSuffix: string): void => {
  const dataStr = `data:application/json;charset=utf-8,${encodeURIComponent(JSON.stringify(data))}`;
  const download = document.createElement('a');
  download.setAttribute('href', dataStr);
  download.setAttribute('download', `${baseFilename}${timeSuffix}.txt`);
  document.body.appendChild(download);
  download.click();
  download.remove();
};

export const downloadPdf = (data: Event[], timeSuffix: string): void => {
  // We cannot change the name of an external package's method...
  // eslint-disable-next-line new-cap
  const doc = new jsPDF();
  const tableHead = data[0] ? Object.keys(data[0]) : [];
  const tableBody: string[][] = [];
  data.map((entry: Event) => tableBody.push(Object.values(entry)));
  autoTable(doc, {
    head: [tableHead],
    body: tableBody,
  });
  doc.save(`${baseFilename}${timeSuffix}.pdf`);
};

export const downloadXlsxCsv = (
  data: Event[],
  format: 'xlsx' | 'csv',
  timeSuffix: string,
): void => {
  const worksheet = XLSX.utils.json_to_sheet(data);
  const workbook = XLSX.utils.book_new();
  XLSX.utils.book_append_sheet(workbook, worksheet, 'Events');
  XLSX.writeFile(workbook, `${baseFilename}${timeSuffix}.${format}`, {
    bookType: format,
  });
};

export const getTimeSuffix = (startTime?: string, endTime?: string): string => {
  // time format: 'yyyy/mm/dd hh:mm:ss'
  if (!startTime || !endTime) {
    return '';
  }
  // handle EventsHeader bug with double spaces,
  // which is not an issue with web server
  const startDate = startTime.replaceAll('  ', ' ').split(' ')[0];
  const startTimestamp = startTime.replaceAll('  ', ' ').split(' ')[1];
  const endTimestamp = endTime.replaceAll('  ', ' ').split(' ')[1];
  return (
    '_'
    + `${startDate.split('/')[0]}-`
    + `${startDate.split('/')[1]}-`
    + `${startDate.split('/')[2]}_`
    + `${startTimestamp.split(':')[0]}`
    + `${startTimestamp.split(':')[1]}to`
    + `${endTimestamp.split(':')[0]}`
    + `${endTimestamp.split(':')[1]}`
  );
};
