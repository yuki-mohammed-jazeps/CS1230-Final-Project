#version 330 core

in vec3 vec_pos;
in vec3 vec_nor;
in vec2 vec_uv;

out vec4 fragcolor;

// Extra credit: texture mapping
uniform bool texturing;
uniform sampler2D tex;
uniform float tex_blend;

// Parallax mapping
uniform bool parallax_frag;
uniform sampler2D normal_map;
uniform sampler2D disp_map;
in mat3 tangent_matrix;

// Object specific color
uniform vec3  ambient;
uniform vec3  diffuse;
uniform vec3  specular;
uniform float shine;

// Global lighting variables
uniform float ka;
uniform float kd;
uniform float ks;

// Texture repeats
uniform float u_repeat;
uniform float v_repeat;

// I realize it's ugly to have so many vectors,
// hopefully that makes this better
uniform vec3  light_directions[8];
uniform vec3  light_positions[8];
uniform vec3  light_colors[8];
uniform int   light_types[8];
uniform float light_angles[8];
uniform float light_penumbras[8];
uniform vec3  light_functions[8];
uniform vec3  camera_pos;

// shadow mapping related
uniform bool do_shadows;
int spotLightNum = 0;  // used to index into below uniforms - this is incremented after calculate_shadow is called
uniform sampler2D shadowMap[4];
uniform mat4 spotLightSpaceMat[4];

float calculate_shadow(float bias)
{
    float shadow = 0.0f;

    // Shadow calculations for spot light ///
    vec4 fragPosLightSpace = spotLightSpaceMat[spotLightNum] * vec4(vec_pos, 1);  // coordinate of world space frag from POV of spot light
    vec3 lightCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;  // get it into clip space
    if (lightCoords.z <= 1.0f) {  // only calc shadow if visible (i.e. if its inside the frustum of perspective proj)

        lightCoords = (lightCoords + 1.0f) / 2.0f ;
        float currentDepth = lightCoords.z;

        // hard shadows
//         float closestDepth = texture(shadowMap[spotLightNum], lightCoords.xy).r;
//         if (currentDepth > closestDepth+bias) {
//            shadow = 1.0;
//         }

        // soft shadows
        int sR = 5;
        vec2 pixelSize = 1.0 / textureSize(shadowMap[spotLightNum], 0);
        for (int y = -sR; y <=  sR; y++) {
            for (int x = -sR; x <= sR; x++) {
                float closestDepth = texture(shadowMap[spotLightNum], lightCoords.xy + vec2(x, y)*pixelSize).r;
                if (currentDepth > closestDepth+bias) {
                    shadow += 1.0;
                }
            }
        }
        shadow /= pow((sR*2+1), 2);
    }

    return shadow;
}

// Parallax mapping
vec2 displace_parallax(vec3 camera_pos) {
  // Get tangent space direction to camera
  mat3 inv_tangent_matrix = transpose(tangent_matrix);
  vec3 to_camera = normalize(inv_tangent_matrix * camera_pos - inv_tangent_matrix * vec_pos);
  to_camera.y *= -1;

  // Number of layers to sample depends on viewing angle
  float angle         = max(0.f, dot(vec3(0.f, 0.f, 1.f), to_camera));
  float sample_layers = (28.f * angle) + 4; // 4 to 32 samples
  float layer_theta   = 1.f / sample_layers;

  // Parallax vector calculation, rise off of texture depends on displacement map
  vec2 parallax_vector = to_camera.xy * 0.1f;
  vec2 coord_theta     = parallax_vector / sample_layers;

  // Step over layers until we find height larger than current height
  vec2  uv_coords  = vec_uv;
  float curr_disp  = texture(disp_map, uv_coords).r;
  float curr_depth = 0.f;

  while (curr_depth < curr_disp) {
    uv_coords  -= coord_theta;
    curr_disp   = texture(disp_map, uv_coords).r;
    curr_depth += layer_theta;
  }

  return uv_coords;
}

// Calculate falloff for spotlights
float calculate_falloff(float theta, float outer, float penumbra) {
  float inner = outer - penumbra;

  if (theta < inner)
    return 1.f;
  if (theta > outer)
    return 0.f;

  return 1.f - (-2.f * pow(((theta - inner) / (outer - inner)), 3.f) +
                 3.f * pow(((theta - inner) / (outer - inner)), 2.f));
}

// Sets color coefficients and light direction for the different kinds of light
bool set_light_info(inout vec3 direction, inout vec3 colors,
  int type, vec3 l_col, vec3 l_dir, vec3 l_pos, vec3 l_fun,
  float angle, float penumbra, vec3 position) {
  // Don't consider area lights or empty lights
  if (type == 3 || type == -1)
    return false;

  // Base color coefficients
  colors = l_col;

  // For directional lights we have no attenuation and we always
  // consider them for color (i.e. return true)
  if (type == 1) {
    direction = normalize(l_dir);
    return true;
  }

  if (type == 0) {
    direction = normalize(position - l_pos);
  } else if (type == 2) {
    vec3 light_direction = normalize(l_dir);
    direction = normalize(position - l_pos);
    // Calculate falloff
    float theta   = acos(dot(light_direction, direction));
    float falloff = calculate_falloff(theta, angle, penumbra);

    // Add to coefficients and continue
    colors *= falloff;
  }

  // Attenuation is done for both points and directionals
  float light_distance = distance(position, l_pos);
  float f_att          = min(1.f, 1 / (l_fun.x +
                                       l_fun.y * light_distance +
                                       l_fun.z * light_distance * light_distance));
  // Early exit if attenuation factor is too small
  if (f_att < 0.0001f)
    return false;

  colors *= f_att;
  return true;
}

void main() {
  vec3 normal    = normalize(vec_nor);
  vec3 to_camera = normalize(camera_pos - vec_pos);
  fragcolor      = vec4(0.f, 0.f, 0.f, 1);

  // Calculate ambient light
  fragcolor += vec4(ka * ambient.x, ka * ambient.y, ka * ambient.z, 0.f);

  // Diffuse light coefficient
  vec3 diff = kd * diffuse;

  // Mix in texture if applicable
  if (texturing) {
    vec2 uv_coords = vec_uv;

    // Parallax
    if (parallax_frag) {
      uv_coords = displace_parallax(camera_pos);

      if(uv_coords.x > u_repeat || uv_coords.y > v_repeat ||
         uv_coords.x < 0.0      || uv_coords.y < 0.0)
        discard;

      normal = texture(normal_map, uv_coords).rgb;
      normal = normalize(normal * 2.0 - 1.0);
      normal = normalize(tangent_matrix * normal);
    }

    vec4 tex_color = texture(tex, uv_coords);

    if (tex_color.w < 0.1)
      discard;

    diff = mix(diff, vec3(tex_color), tex_blend);
    fragcolor.w = tex_color.w;
  }

  // Specular light coefficient
  vec3 spec = ks * specular;

  // For each light
  for (int i = 0; i != 8; ++i) {
    // Skip empty lights
    if (light_types[i] == -1) {
      continue;
    }

    vec3 to_light    = vec3(0.f, 0.f, 0.f);
    vec3 base_colors = vec3(0.f, 0.f, 0.f);

    // Set the light type dependent information
    bool consider = set_light_info(to_light, base_colors,
      light_types[i], light_colors[i], light_directions[i],
      light_positions[i], light_functions[i], light_angles[i],
      light_penumbras[i], vec_pos);

    if (!consider)
      continue;

    if (dot(to_light, normal) >= 0.f || dot(to_camera, normal) < 0.f)
      continue;

    // Calculate diffuse light
    float dot_prod_d   = max(dot(-to_light, normal), 0.f);
    vec3  curr_diffuse = diff * dot_prod_d;

    // Calculate specular light
    float dot_prod_s = max(dot(reflect(to_light, normal), to_camera), 0.f);
    if (shine > 0.f || (shine == 0.f && dot_prod_s > 0.f)) {
      dot_prod_s = pow(dot_prod_s, shine);
    }
    vec3 curr_spec = dot_prod_s * spec;

    // Shadow calculation
    float shadow = 0;  // no shadow by default
    if (light_types[i] == 2 && do_shadows) {  // only calculate shadow if spot light
        float bias = max(0.0005 * (1.0 - dot(-to_light, normal) ), 0.00005f);  // bias to avoid self-shadowing (tinker with max and min val)
        shadow =  calculate_shadow(bias);
        spotLightNum += 1;
    }

    // Put it all together
    fragcolor += vec4(base_colors * (1 - shadow) * (curr_diffuse + curr_spec), 0.f);
  }
}
