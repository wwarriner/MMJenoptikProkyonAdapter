function out = image_to_hex_sequence(image_file)

if nargin < 1
    image_file = fullfile(fileparts(mfilename("fullpath")), "example.bmp");
end

im = imread(image_file);
frames = min(size(im, [1 2]));

hex = strings(frames, 1);
counts = zeros(frames, 1);
for i = 1 : frames
    shift = i - 1;
    im_copy = im;
    for dim = 1 : 2
        im_copy = circshift(im_copy, shift, dim);
    end
    [hex(i), counts(i)] = convert_image_to_cpp_vector_literal(im_copy);
end

values = strip(hex, "right", ";");
values = values + "," + newline;
values = strjoin(values, "");
values = strip(values, "right", newline);
values = strip(values, "right", ",");
out = "img_seq{" + values + "};";

end

