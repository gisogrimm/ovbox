function plotstat(fname)
  csLabels = {'JuliaGiso','Marthe','Frauke','Hille','Claas'};
  jitterbuf = 20;
  hwdelay = 11;
  csnd = 340;
  pkg load statistics
  [tmp,txt] = system(['cat ',fname,'|grep -e latency']);
  c = textscan(txt,'%[^[][4464] latency %d min=%fms, mean=%fms, max=%fms');
  cdates = c{1};
  mcaller = c{2};
  mmin = c{3};
  mmean = c{4};
  mmax = c{5};
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
  plot_lat_and_jitter_matrix( pinglat+pinglat', jitter+jitter', ...
			      pinglat, ...
			      jitter, csLabels(callers(idx)+1));
  title([fname,' via server'],'interpreter','none');
  saveas(gcf,[fname,'_ping_server.png'],'png');
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
  % peer2peer
  [vcid1,vcid2,vmin,vmean,vmax] = get_peer_lat( fname );
  mlat = zeros(numel(csLabels),numel(csLabels))+inf;
  mjit = mlat;
  mnum = mlat;
  for cid1=[1:numel(csLabels)]-1
    for cid2=setdiff([1:numel(csLabels)]-1,cid1)
      idx = find((vcid1==cid1).*(vcid2==cid2));
      mnum(cid1+1,cid2+1) = numel(idx);
      if ~isempty(idx)
	mlat(cid1+1,cid2+1) = nanmean(vmean(idx));
	mjit(cid1+1,cid2+1) = nanmean(vmax(idx)-vmean(idx));
      end
    end
  end
  mnum
  mlat(find(~isfinite(mlat))) = nan;
  mjit(find(~isfinite(mjit))) = nan;
  figure
  plot_lat_and_jitter_matrix( mlat, mjit, ...
			      nanmean(mlat), ...
			      nanmean(mjit), csLabels);
  title([fname,' peer-to-peer'],'interpreter','none');
  saveas(gcf,[fname,'_ping_p2p.png'],'png');
  
function [vcid1,vcid2,vmin,vmean,vmax] = get_peer_lat( fname )
  [tmp,txt] = system(['cat ',fname,'|grep -e peerlat']);
  c1 = textscan(txt,'%[^[][4464] peerlat %d-%d min=%fms, mean=%fms, max=%fms');
  cdates = c1{1};
  vcid1 = c1{2};
  vcid2 = c1{3};
  vmin = c1{4};
  vmean = c1{5};
  vmax = c1{6};


function plot_lat_and_jitter_matrix( mlat, mjit, vlat, vjit, labels )
  imdata = mlat;
  idx = 1:size(mlat,1);
  for k=idx
    imdata(k,k) = 0;
  end
  imagesc(imdata);

  set(gca,'clim',[0,90]);
  hold on;
  map = colormap('jet');
  map(1,:) = ones(1,3);
  colormap(map);
  for kx=1:size(mlat,2)
    plot(kx+[0.5,0.5],[0.5,size(mlat,1)+0.5],'k-');
    hold on
    plot([0.5,size(mlat,1)+0.5],kx+[0.5,0.5],'k-');
    text( kx, size(mlat,1)+0.85,sprintf('%1.1f ms\n(%1.1f ms)',vlat(kx),vjit(kx)),...
	  'HorizontalAlignment','center',...
	  'FontSize',12);
    for ky=1:size(mlat,1)
      if kx ~= ky
	fmt = '%1.1f ms\n(%1.1f ms)';
	text(kx,ky,sprintf(fmt,mlat(ky,kx),mjit(ky,kx)),...
	     'HorizontalAlignment','center',...
	     'FontSize',14);
	hold on
      end
    end
  end
  text(size(mlat,1)*0.5+0.5,size(mlat,1)+1.25,...
       'average ping latency (jitter):',...
       'HorizontalAlignment','center',...
       'FontSize',12);
  set(gca,'XLim',[0.5,size(mlat,1)+0.5],...
	  'XTick',1:size(mlat,1),...
	  'XTickLabel',labels,...
	  'YLim',[0.5,size(mlat,1)+1.5],...
	  'YTick',1:size(mlat,1),...
	  'YTickLabel',labels,...
	  'YDir','normal')
