function data = plot_matrix(mySerial)
  nsamples = 100;       % first get the number of samples being sent
  data = zeros(nsamples,4);               % two values per sample:  ref and actual
  for i=1:nsamples
    data(i,:) = fscanf(mySerial,'%d\t%d\t%d\t%d\r\n'); % read in data from PIC32; assume ints, in mA
    times(i) = (i-1);                % 0.2 ms between samples
  end
  if nsamples > 1						        
    stairs(times,data(:,1:4));            % plot the reference and actual
  else
    fprintf('Only 1 sample received\n')
    disp(data);
  end
  title(sprintf('Average error: %5.1f mA',score));
  ylabel('Current (mA)');
  xlabel('Time (ms)');  
end
