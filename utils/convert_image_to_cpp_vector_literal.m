function [out, count] = convert_image_to_cpp_vector_literal(im)

im = flip(im, 3);
sz = size(im);
im = cat(3, im, zeros(sz(1:2)));
sz = size(im);
bin = dec2hex(im);
bin = string(bin);
bin = reshape(bin, sz);
bin = permute(bin, [3 2 1]);
out = to_string(bin(:));
count = numel(bin);

end


function s = to_string(bin)

s = sprintf("0x%s,", bin);
s = "img{" + strip(s, "right", ",") + "};";

end