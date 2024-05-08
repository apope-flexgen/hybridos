import { Observable } from 'rxjs';
import { AddLayout } from './dto/layout.dto';
import { LayoutsResponse } from './responses';

export interface ILayoutsService {
  getLayouts(): Promise<Observable<LayoutsResponse>>;
  postLayouts(data: AddLayout): Promise<LayoutsResponse>;
}
