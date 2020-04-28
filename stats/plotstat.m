function plotstat(fname)
  [tmp,txt] = system(['cat ',fname,'|grep -e latency']);
  c = textscan(txt,'%[^[][4464] latency %d min=%fms, mean=%fms, max=%fms');
  jitterbuf = 20;
  hwdelay = 11;
  csnd = 340;
  cdates = c{1};
  mcaller = c{2};
  mmin = c{3};
  mmean = c{4};
  mmax = c{5};
  csLabels = {'JuliaGiso','Marthe','Frauke','Hille','Claas'};
  callers = unique(mcaller);
  dates = unique(cdates);
  mdata = nan*zeros([numel(dates),numel(callers),3]);
  for k=1:numel(cdates)
    idxdate = strmatch(cdates{k},dates,'exact');
    idxcaller = find(callers==mcaller(k));
    mdata(idxdate,idxcaller,:) = [mmin(k),mmean(k),mmax(k)];
  end
  idx = find(~isnan(sum(mdata(:,:,2),2)));
  mdata = mdata(idx,:,:);
  idx = find(callers<=4);
  map = lines(numel(csLabels));
  srv = mdata(:,idx,3)-mdata(:,idx,2);
  %srv = prod(srv,2).^(1/size(srv,2));
  srv = min(srv,[],2);
  figure
  phserv = plot(srv,'k-','linewidth',5,'Color',ones(1,3)*0.8);
  hold on
  ph2 = plot(squeeze(mdata(:,idx,2)),'-','linewidth',2);
  for k=1:numel(ph2)
    set(ph2(k),'Color',map(callers(idx(k))+1,:));
  end
  ph = plot(squeeze(mdata(:,idx,3)),'-');
  for k=1:numel(ph)
    set(ph(k),'Color',map(callers(idx(k))+1,:));
  end
  imin = 0;
  imax = numel(dates);
  for k=1:numel(idx)
    tmp = find(~isnan(squeeze(mdata(:,k,2))));
    imin = max(imin,min(tmp));
    imax = min(imax,max(tmp));
  end
  xlim([imin,imax]);
  ylim([0,140]);
  csLeg = {'server jitter'};
  csLeg = [csLeg,csLabels(callers(idx)+1)];
  legend([phserv;ph2],csLeg);
  pinglat = [];
  lat = [];
  jitter = [];
  for k=1:numel(idx)
    pinglat(end+1) = mean(mdata(imin:imax,k,2));
    lat(end+1) = (mean(mdata(imin:imax,k,2))+jitterbuf+hwdelay)*0.5;
    jitter(end+1) = mean(mdata(imin:imax,k,3)-mdata(imin:imax,k,2));
  end
  mlat = (lat+lat');
  %.* (1-eye(numel(lat)))
  ylabel('ping network latency / ms');
  xlabel('time / minutes');
  saveas(gcf,[fname,'_latency.png'],'png');
  figure
  imagesc(mlat.*(1-eye(size(mlat,1))));
  set(gca,'clim',[10,90]);
  hold on;
  map = colormap('jet');
  map(1,:) = ones(1,3);
  colormap(map);
  for kx=1:size(mlat,2)
    plot(kx+[0.5,0.5],[0.5,size(mlat,1)+0.5],'k-');
    hold on
    plot([0.5,size(mlat,1)+0.5],kx+[0.5,0.5],'k-');
    text( kx, size(mlat,1)+0.7,sprintf('%1.1f (%1.1f) ms',pinglat(kx),jitter(kx)),...
	  'HorizontalAlignment','center',...
	  'FontSize',12);
    for ky=1:size(mlat,1)
      fmt = '%1.1f ms\n%1.1f m';
      if kx==ky
	fmt = '(%1.1f ms)\n(%1.1f m)';
      end
      text(kx,ky,sprintf(fmt,mlat(ky,kx),0.001*mlat(ky,kx)*csnd),...
	   'HorizontalAlignment','center',...
	   'FontSize',14);
      hold on
    end
  end
  text(size(mlat,1)*0.5+0.5,size(mlat,1)+1,...
       'average ping latency (jitter) from server:',...
       'HorizontalAlignment','center',...
       'FontSize',12);
  set(gca,'XLim',[0.5,size(mlat,1)+0.5],...
	  'XTick',1:size(mlat,1),...
	  'XTickLabel',csLabels(callers(idx)+1),...
	  'YLim',[0.5,size(mlat,1)+1.2],...
	  'YTick',1:size(mlat,1),...
	  'YTickLabel',csLabels(callers(idx)+1),...
	  'YDir','normal')
  title(fname,'Interpreter','none');
  saveas(gcf,[fname,'_delaymatrix.png'],'png');
	  