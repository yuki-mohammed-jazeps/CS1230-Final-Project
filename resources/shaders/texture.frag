#version 330 core

// Create a UV coordinate in variable
in vec2 vec_uv;

// Add a sampler2D uniform
uniform sampler2D tex;

// Add a bool on whether or not to filter the texture
uniform bool should_invert;
uniform bool should_blur;
uniform bool should_grayscale;
uniform bool should_sharpen;

out vec4 fragcolor;

// Get grayscale using the perception method
float get_grayscale(vec4 color) {
  return 0.299 * color.x + 0.587 * color.y + 0.114 * color.z;
}

// Get average from surrounding pixels
vec4 blur() {
  vec4 ret = vec4(0.f);

  // This will be the unit value of one texel step in any direction
  vec2 inverse_texture = 1.f / textureSize(tex, 0);

  // Get all texels in 5x5 neighborhood
  for (int i = -2; i != 3; ++i) {
    for (int j = -2; j != 3; ++j) {
      vec4 neighbor = texture(tex, vec_uv + inverse_texture * vec2(i, j));
      ret          += neighbor * 0.04f;
    }
  }

  return ret;
}

// Remove influence from surrounding pixels
vec4 sharpen() {
  vec4 ret = vec4(0.f);

  // This will be the unit value of one texel step in any direction
  vec2 inverse_texture = 1.f / textureSize(tex, 0);

  // Get all texels in 3x3 neighborhood
  for (int i = -1; i != 2; ++i) {
    for (int j = -1; j != 2; ++j) {
      vec4 neighbor = texture(tex, vec_uv + inverse_texture * vec2(i, j));
      if (i == 0 && j == 0)
        ret += neighbor *  1.888888f;
      else
        ret += neighbor * -0.111111f;
    }
  }

  return ret;
}

void main()
{
    // Set fragColor using the values of the neighboring texels,
    // or just from sampler2D at the UV coordinate if not filtering
    if (should_blur || should_sharpen) {
      if (should_blur)
        fragcolor = blur();
      if (should_sharpen)
        fragcolor = sharpen();
    } else {
      fragcolor = texture(tex, vec_uv);
    }

    // Invert fragcolor's r, g, and b color channels if invert bool is true
    if (should_invert)
      fragcolor = vec4(1.f, 1.f, 1.f, 1.f) - fragcolor;

    // Turn to grayscale using
    if (should_grayscale){
      float gray = get_grayscale(fragcolor);
      fragcolor = vec4(gray, gray, gray, 1.f);
    }

    fragcolor.w = 1.f;
}
