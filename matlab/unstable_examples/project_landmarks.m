function [ measurements ] = project_landmarks( pose, landmarks, K )
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here

    import gtsam.*;

    camera = SimpleCamera(pose,K);    
    measurements = Values;
    
    for i=1:size(landmarks)-1
        z = camera.project(landmarks.at(symbol('l',i)));
        
        % check bounding box
        if z.x < 0 || z.x > 1280
            continue
        elseif z.y < 0 || z.y > 960
            continue
        end
           
        measurements.insert(symbol('z',i),z);
    end
    
%     persistent h;
%     
%     if isempty(h)
%         h = figure();
%     else
%         figure(h);
%     end
%     clf;
    
    if measurements.size == 0
        return
    end
    
    cla;
    plot2DPoints(measurements,'*g');
    
    axis equal;
    axis([0 1280 0 960]);
    
    set(gca, 'YDir', 'reverse');
    xlabel('u (pixels)');
    ylabel('v (pixels)');
    set(gca, 'XAxisLocation', 'top');
    

end

